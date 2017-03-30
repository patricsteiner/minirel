#include <stdio.h>
#include "bf.h"
#include "minirel.h"
#include "lru.h"
#include "hashtable.h"
#include "freelist.h"

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
	fl = fl_init(BF_MAX_BUFS); //buffer entries malloc in the fl_function
	lru = lru_init();
	ht = ht_init(BF_HASH_TBL_SIZE);
}

/*
 * Unlike BF_GetBuf(), this function is used in situations where a new page is 
 * added to a file. Thus, if there already exists a buffer page in the pool associated
 * with a PF file descriptor and a page number passed over in the buffer control 
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
	BFhash_entry* e = ht_get(ht, &fpage->fd, &fpage->pageNum);
	if (e != NULL) {
		return BFE_PAGEINBUF; /* it is a new page, so it must not be in buffer yet */
	}
	BFpage* page = fl_give_one(fl);
	if (page == NULL) { /* there is no free page, need to replace one (aka find victim) */
		page = lru_remove(lru);
	}

	page->count = 1;
	page->dirty = FALSE;
	page->prevpage = NULL;
	page->nextpage = NULL;
	/*page->fpage = &&fpage; ?????? TODO*/
	page->pageNum = bq.pagenum;
	page->fd = bq.fd; /* TODO or unixfd? */

	ht_add(ht, page);
	return lru_add(lru, page);
}

/*
 * Returns a PF page in a memory block pointed to by *fpage
 * Dev : Antoine
 */
int BF_GetBuf(BFreq bq, PFpage **fpage){
	BFhash_entry* h_entry;
	BFpage* bfpage_entry;

	h_entry = ht_get(ht, bq->fd, bq->pagenum);

	if(h_entry != NULL){
		h_entry->count += 1;
	}else{
		//get a empty freelist BFpage, add it to the hashtable and ti the lru list		
		bfpage_entry = fl_give_one(fl);
		if(bfpage_entry != NULL){
			//first add in the lru and then in the hashtable
			if(lru_add(lru, bfpage_entry) == 0 && ht_add(ht, bfpage_entry) == 0){
			}else{
				//error handling : impossible to add the entry
				// error code add into pf.h (-9)
				return PFE_GETBUF;
			}

			
		}else{
			//remove a page from the lru list
			//add it to the freelist (the page will be cleared)
			//remove from the hashtable
			//add a page to the lru list (with the right information) with fl_give_one
			//notify the hash table from this change
			

		}	
	}

	return 0;
	// a memory block is a BFpage
	// The bufferRequest is composed by (fd, unixfd, pagenum, dirty) 
	/*	ALGO
	if fd in LRU List (check with the hashtable)
		pincount += 1
	else
		allocate a new buffer page (by page replacement if no free in the freelist)
		TODO : add a function in freelist.c freeSpaceAvailable(Freelist* fl)
	 	read the file page into fpage.bufferpage
	 	pincount set to 1
	 	dirty set to FALSE
		other fields
	the page then become the most recently used
	return BFE_OK if correct


	 */
}

/*
 * This function unpins the page whose identification is passed over in the 
 * buffer control block bq. This page must have been pinned in the buffer already.
 * Unpinning a page is carried out by decrementing the pin count by one. 
 * This function returns BFE_OK if the operation is successful, an error condition otherwise.
 * Dev : Patric
 */
int BF_UnpinBuf(BFreq bq){
	BFPage* page = ht_get(ht, bq.fd, bq.pagenum);
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
 * Dev : Patric
 */
int BF_TouchBuf(BFreq bq){
	BFPage* page = ht_get(ht, bq.fd, bq.pagenum);
	if (page == NULL || page->count < 1) { /* must be pinned */
		return -1; /*TODO return what?*/
	}
	pq.dirty = true; /*TODO needed? or just page dirty?*/
	page->dirty = TRUE;
	return lru_mtu(lru, page);
}

/*
 * Dev : Paul
 */
int BF_FlushBuf(int fd){
	return 0;
}

/*
 * Dev : Paul
 */
void BF_ShowBuf(void){}

