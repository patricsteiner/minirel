#include <stdio.h>
#include "bf.h"
#include "minirel.h"
#include "hashtable.h"
#include "freelist.h"

/*
 * Init the BF layer
 * Creates all the buffer entries and add them to the freelist
 * Also init the hashtable
 * Dev : Antoine
 */
void BF_Init(void){
	Freelist* fl = fl_init(BF_MAX_BUFS);
	Hashtable* ht = ht_init(BF_HASH_TBL_SIZE);
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
	return 0;
	// a memory block is a BFpage
	// The bufferRequest is composed by (fd, unixfd, pagenum, dirty) 
	/*	ALGO
	if fd in LRU List
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