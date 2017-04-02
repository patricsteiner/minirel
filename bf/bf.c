#include <stdio.h>
#include <unistd.h>
#include "bfheader.h"
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
    page->dirty=TRUE;  //???????????? TRUE or bq.dirty? /////////////////////////////////////////////////////////////
    
    /* page has to be head of the list */
    return lru_mtu(lru, page); 
	
	return 0;
}

/*
 * Dev : Paul
 */
int BF_FlushBuf(int fd){
	if (lru->tail==NULL) return BFE_PAGENOTINBUF; //empty list case

	//we start by checking if tail is page of this file 
	BFpage *pt= lru->tail;
	int ret; /* used to recuperate return values */

	do{
	     if ( pt->fd==fd){

		 if(pt->count==0){ 

			/* to remove from the lru list, change pointer is enough */ 
			if (pt != lru->head){
	  
				if(pt!=lru->tail) {
					pt->prevpage->nextpage=pt->nextpage; 
					pt->nextpage->prevpage=pt->prevpage;
				 }
			
				else{	
					pt->prevpage->nextpage=NULL;/*tail removed ==> the page before tail becomes the tail	*/	
					lru->tail=pt->prevpage;
				}
			}
			else{
				if(pt!=lru->tail) {
						pt->nextpage->prevpage=NULL;// head removed ==> next page becomes head
						lru->head=pt->nextpage;
				 }
				else{/* the head is the tail */
					lru->head=NULL;
					lru->tail=NULL;
				}
			}


			/************  if page is dirty, we write it on the disk             *********************************/
			if(pt->dirty){
				ret=pwrite(pt->unixfd,pt->fpage->pagebuf, PAGE_SIZE, PAGE_SIZE*((pt->pagenum)-1));

				if(ret<0){
					perror("impossible to write the file");
				}
				if(PAGE_SIZE>ret){
					return BFE_INCOMPLETEWRITE;
				}			

			}
			/************ page is removed, next step add it to the free list ********************************/

			pt->nextpage=NULL;
			pt->prevpage=NULL;
			lru->number_of_page-=1;
			fl_add(fl, pt);
		}

		else { /* the page is still pinned */
 			return BFE_PAGEPINNED; 
		}
		
	     pt=pt->prevpage;
	}while(pt!=NULL); /*stop the loop after the head*/

	return BFE_OK; /*every page of the file which were in the LRU list, are now in the free list*/
	
  
}

/*
 * Dev : Paul
 */
int BF_ShowBuf(void){
	return lru_print(lru);

}
	

