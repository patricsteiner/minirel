#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "minirel.h"
#include "bfUtils.h"
#include "bf.h"

/*
 * globally used data structures: freelist, hashtable and lru
 */
Freelist* fl;
LRU* lru;
Hashtable* ht;

/*
 * Init the BF layer
 * Creates all the buffer entries and add them to the freelist
 * Also init the hashtable
 * Dev : Antoine
 */
void BF_Init(void){
	/*
	 * TODO How to do the error_handling ? 
	 */
	fl = fl_init(BF_MAX_BUFS); /*buffer entries malloc in the fl_function*/
	lru = lru_init();
	ht = ht_init(BF_HASH_TBL_SIZE);
}

/*
 * -find a victim, flush it in disk dive if it is dirty 
 * -remove it from hastable and add a new page in free
 * -used in BF_GetBuf and BF_AllocBuf
 * Dev:Paul
 */
int BF_FlushPage(LRU* lru){
	BFpage* victim;
	int res; 
			
	/*try to find a victim */
	res = lru_remove(lru, &victim);	
        /*no victim found */	
	if(res != 0){
		return res;
	} 

	/*victim found */	
	/*victim dirty : try to flush it, trhow error otherwise */
	if(victim->dirty == TRUE){
		if(pwrite(victim->unixfd, victim->fpage.pagebuf, PAGE_SIZE, ((victim->pagenum))*PAGE_SIZE) != PAGE_SIZE){
			return BFE_INCOMPLETEWRITE;
		}
	}
	
	/*removes victim */ 
	res=ht_remove(ht, victim->fd, victim->pagenum);

	if(res != 0){
		printf("not found in hashtable \n");
		return res;
	} 
	res = fl_add(fl, victim);	
	if(res != 0){
		printf("fl add \n");
		return res;
	} 
}	

/*
 * Unlike BF_GetBuf(), this function is used in situations where a new page is 
 * added to a file. Thus, if there already exists a buffer page in the pool associated
 * with a PF file ² and a page number passed over in the buffer control 
 * block bq, a PF error code must be returned. Otherwise, a new buffer page should 
 * be allocated (by page replacement if there is no free page in the buffer pool). 
 * Then, its pin count is set to one, its dirty flag is set to FALSE, other appropriate
 * fields of the BFpage structure are set accordingly, and the page becomes the most
 * recently used. Note that it is not necessary to read anything from the file into 
 * the buffer page, because a brand new page is being appended to the file. This function
 * returns BFE_OK if the operation is successful, an error condition otherwise.
 * Dev : Patric
 */
int BF_AllocBuf(BFreq bq, PFpage **fpage){
	BFhash_entry* e;
	BFpage* page;
	int res;

	e = ht_get(ht, bq.fd, bq.pagenum);
	if (e != NULL) {
		return BFE_PAGEINBUF; /* it is a new page, so it must not be in buffer yet */
	}
	page = fl_give_one(fl);
	if (page == NULL) { /* there is no free page, need to replace one (aka find victim) */
		/* remove a victim in lru and add a new page in the freelist */
		res = BF_FlushPage(lru);
		if(res != 0){
			return res;
		} 
		/* we use this new page */
		page = fl_give_one(fl);	
	}

	page->count = 1;
	page->dirty = FALSE;
	page->prevpage = NULL;
	page->nextpage = NULL;
	page->pagenum = bq.pagenum;
	page->fd = bq.fd;
	page->unixfd = bq.unixfd;

	*fpage = &page->fpage;

	ht_add(ht, page);
	return lru_add(lru, page);
}

/*
 * Returns a PF page in a memory block pointed to by *fpage
 * Dev : Antoine
 */
int BF_GetBuf(BFreq bq, PFpage** fpage){
	BFhash_entry* h_entry;
	BFpage* bfpage_entry;
	BFpage* victim;
	int res;

	h_entry = ht_get(ht, bq.fd, bq.pagenum);

	/*page already in buffer */
	if(h_entry != NULL){
		printf( " ce que me donne la ht  %d , %d \n" , h_entry->bpage->fd, h_entry->bpage->pagenum);
		h_entry->bpage->count += 1;
		(*fpage) = &(h_entry->bpage->fpage);
		return BFE_OK;
	}

	/*page not in buffer */
	bfpage_entry = fl_give_one(fl);

	/*No more  place in the buffer <=> No more freespace */
	if( bfpage_entry == NULL){
		
		/* remove a victim in lru and add a page in the freelist */
		res=BF_FlushPage(lru);
		if(res != BFE_OK) return res;

		bfpage_entry = fl_give_one(fl);
	}
	
	/*set the correct parameters */
	bfpage_entry->count = 1;
	bfpage_entry->dirty = FALSE;
	bfpage_entry->fd = bq.fd;
	bfpage_entry->pagenum = bq.pagenum;
	bfpage_entry->unixfd = bq.unixfd;

	/* add page to LRU and HT */
	if(lru_add(lru, bfpage_entry) == BFE_OK && ht_add(ht, bfpage_entry) == BFE_OK){
	}else{return BFE_PAGENOTOBUF;}

	/*try to read the file asked */
	if(pread(bq.unixfd, bfpage_entry->fpage.pagebuf, PAGE_SIZE, ((bq.pagenum))*PAGE_SIZE) == -1){
		return BFE_INCOMPLETEREAD;
	}

	/*value returned to the user */
	(*fpage) = &(bfpage_entry->fpage);
	return BFE_OK;
}

/*
 * This function unpins the page whose identification is passed over in the 
 * buffer control block bq. This page must have been pinned in the buffer already.
 * Unpinning a page is carried out by decrementing the pin count by one. 
 * This function returns BFE_OK if the operation is successful, an error condition otherwise.
 * Dev : Patric
 */
int BF_UnpinBuf(BFreq bq){
	BFhash_entry* ht_entry;
	
	ht_entry = (ht_get(ht, bq.fd, bq.pagenum)); 
	if (ht_entry == NULL ) { /* must be pinned */
		return BFE_PAGENOTINBUF;
	}
	if(ht_entry->bpage->count<1) return BFE_UNPINNEDPAGE;
	ht_entry->bpage->count = ht_entry->bpage->count - 1;
	return BFE_OK;
}

/*
 * This function marks the page identified by the buffer control block bq as dirty.
 * The page must have been pinned in the buffer already. The page is also made the
 * most recently used by moving it to the head of the LRU list. This function
 * returns BFE_OK if the operation is successful, an error condition otherwise.
 * Dev : Paul
 */
int BF_TouchBuf(BFreq bq){
   BFhash_entry* ht_entry;
     
    /* pointer on the page is get by using hastable */
    ht_entry = (ht_get(ht, bq.fd, bq.pagenum));

    /* page has to be pinned */
    if(ht_entry == NULL) return BFE_HASHNOTFOUND;
    if(ht_entry->bpage == 0) return BFE_UNPINNEDPAGE;
    
    /* page is marked as dirty */
    ht_entry->bpage->dirty = TRUE;  
    /* page has to be head of the list */
    return lru_mtu(lru, ht_entry->bpage);
}

/*
 * Writes all the pages with given fd to disk.
 * Dev : Paul
 */
int BF_FlushBuf(int fd){
	int cpt = 0;
	BFpage* pt;
	BFpage* prev; /* when the page will be add in free list, her prevpage pointer will be set to null, we need this variable to copy it before*/
	int ret; /* used to recuperate return values */
	
	/* start checking with the tail, until the head */
	if (lru->tail == NULL) return BFE_OK; /*empty list case*/
	pt = lru->tail;
	do {
	     if ( pt->fd == fd){
		 if(pt->count == 0){ 
			/* to remove from the lru list,to change pointer is enough */ 
			if (pt != lru->head){
				if(pt != lru->tail) {
					pt->prevpage->nextpage = pt->nextpage; 
					pt->nextpage->prevpage = pt->prevpage;
				 }
				else{	
					pt->prevpage->nextpage = NULL;/*tail removed ==> the page before tail becomes the tail	*/	
					lru->tail=pt->prevpage;
				}
			}
			else{
				if(pt != lru->tail) {
						pt->nextpage->prevpage = NULL;/* head removed ==> next page becomes head */
						lru->head=pt->nextpage;
				 }
				else{/* the head is the tail */
					lru->head = NULL;
					lru->tail = NULL;
				}
			}
			/* if page is dirty, we write it on the disk */
			if (pt->dirty){
				ret = pwrite(pt->unixfd,pt->fpage.pagebuf, PAGE_SIZE, PAGE_SIZE*((pt->pagenum)));

				if (ret < 0){
					printf("unix \n");
					return BFE_UNIX;
				}
				if (PAGE_SIZE > ret){
					printf("incomp \n");
					return BFE_INCOMPLETEWRITE;
				}			
			}
			/* page is removed, next step add it to the free list */
			lru->number_of_page-=1;
			prev = pt->prevpage;
			ht_remove(ht, pt->fd, pt->pagenum);
			fl_add(fl, pt);
		}
		else { /* the page is still pinned */
 			return BFE_PINNEDPAGE; 
		}
	}
	pt = prev;
	} while (pt != NULL); /*stop the loop after the head*/

	return BFE_OK; /*every page of the file which were in the LRU list, are now in the free list*/
}

/*
 * prints the content of the lru list
 * Dev : Paul
 */
void BF_ShowBuf(){
	lru_print(lru);
}