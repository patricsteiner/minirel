#ifndef __FREELIST_H__
#define __FREELIST_H__

#include "bfHeader.h"

typedef struct Freelist {
	BFpage*			head;			/* first el of the Freelist */
	unsigned int 	size;			/* current size 			*/		
	unsigned int 	max_size;		/* max_size allowed 		*/
} Freelist;

/*
 * Init the free list with pointers toward empty BFpages as entries
 * Returns a freelist pointers
 */
Freelist* fl_init(int max_size);

/*
 * Returns a pointer toward a free BFpage and unlink it from the fl
 * its actually remove the first el 
 * if no more elem, return null
 */
BFpage* fl_give_one(Freelist* fl);

/*
 * Clear the data into the BFpage and add it to the list
 * return 0 if no error
 */
int fl_add(Freelist* fl, BFpage* BFpage);

/*
 * free the freelist
 */
int fl_free(Freelist* fl);

/*
 * Print the freelist (actual size)
 */
void fl_print(Freelist* fl);

#endif