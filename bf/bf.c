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
 * Dev : Patric
 */
int BF_AllocBuf(BFreq bq, PFpage **fpage){
	return 0;
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
 * Dev : Patric
 */
int BF_UnpinBuf(BFreq bq){
	return 0;
}

/*
 * Dev : Patric
 */
int BF_TouchBuf(BFreq bq){
	return 0;
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