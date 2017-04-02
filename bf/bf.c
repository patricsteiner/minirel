#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "minirel.h"
#include "bfUtils.h"
#include "bf.h"

/*
 * Constants freelist, hashtable, lru
 * Waiting for an answer of the teacher (question was : can we add some code into the headers he provided us ?)
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
	 * How to do the error_handling ? 
	 */
	fl = fl_init(BF_MAX_BUFS); /*buffer entries malloc in the fl_function*/
	lru = lru_init();
	ht = ht_init(BF_HASH_TBL_SIZE);

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
	/* PFpage needs to be filled, the params pass through bq : ht_get(ht, bq.fd, bq.pagenum)
	 * pedantic : variables needs to be declared and assigned in different lines
	 */
	BFhash_entry* e;
	BFpage* page;
	BFpage* victim;
	int res;

	e = ht_get(ht, bq.fd, bq.pagenum);
	/* e = ht_get(ht, &fpage->fd, &fpage->pageNum);*/
	if (e != NULL) {
		return BFE_PAGEINBUF; /* it is a new page, so it must not be in buffer yet */
	}
	page = fl_give_one(fl);
	if (page == NULL) { /* there is no free page, need to replace one (aka find victim) */
		res = lru_remove(lru, &victim);
		/*no victim found */	
		if(res != 0){return res;} 
	}

	page->count = 1;
	page->dirty = FALSE;
	page->prevpage = NULL;
	page->nextpage = NULL;
	/*page->fpage = &&fpage; ?????? TODO*/
	page->pagenum = bq.pagenum;
	page->fd = bq.fd; /* TODO or unixfd? */

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
		h_entry->bpage->count += 1;
		(*fpage) = &(h_entry->bpage->fpage);
		return BFE_OK;
	}

	/*page not in buffer */
	bfpage_entry = fl_give_one(fl);

	/*No more  place in the buffer <=> No more freespace */
	if( bfpage_entry == NULL){
		/*try to find a victim */
		res = lru_remove(lru, &victim);	
		/*no victim found */	
		if(res != 0){return res;} 

		/*victim found */	
		/*victim dirty : try to flush it, trhow error otherwise */
		if(victim->dirty == TRUE){
			if(pwrite(victim->unixfd, victim->fpage.pagebuf, PAGE_SIZE, ((victim->pagenum)-1)*PAGE_SIZE) != PAGE_SIZE){
				return BFE_INCOMPLETEWRITE;
			}
		}

		/*remove victim */
		ht_remove(ht, victim->fd, victim->pagenum); /*not sure what to pass to ht_remove */
		fl_add(fl, victim);
		
		bfpage_entry = fl_give_one(fl);
	}
	
	/* if space available in buffer(add page to LRU and HT) */
	if(lru_add(lru, bfpage_entry) == BFE_OK && ht_add(ht, bfpage_entry) == BFE_OK){
	}else{return BFE_PAGENOTOBUF;}

	/*try to read the file asked */
	if(pread(bq.unixfd, bfpage_entry->fpage.pagebuf, PAGE_SIZE, ((bq.pagenum)-1)*PAGE_SIZE) == -1){
		return BFE_INCOMPLETEREAD;
	}

	/*set the correct parameters */
	bfpage_entry->count = 1;
	bfpage_entry->dirty = FALSE;
	bfpage_entry->fd = bq.fd;
	bfpage_entry->pagenum = bq.pagenum;
	bfpage_entry->unixfd = bq.unixfd;

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
	BFpage* page;
	page = (ht_get(ht, bq.fd, bq.pagenum))->bpage;
	if (page == NULL || page->count < 1) { /* must be pinned */
		return -1; /*TODO return what?*/
	}
	page->count = page->count - 1;
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
    BFpage* page;

    /* pointer on the page is get by using hastable */
    page=(ht_get(ht, bq.fd, bq.pagenum))->bpage;

    /* page has to be pinned */
    if(page==NULL) return BFE_HASHNOTFOUND;
    if(page->count==0) return BFE_UNPINNEDPAGE;
    
    /* page is marked as dirty */
    page->dirty=TRUE;  /*???????????? TRUE or bq.dirty? */
    /* page has to be head of the list */
    return lru_mtu(lru, page);
}

/*
 * Dev : Paul
 */
int BF_FlushBuf(int fd){
	return 1;
}

/*
 * Dev : Paul
 * remark : in bf.h, the proto is void BF_ShowBuf(void)
 */
void BF_ShowBuf(){
	/*return lru_print(lru);*/

}


