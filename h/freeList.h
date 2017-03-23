#include "minirel.h"

/******************************************************************************/
/*   Type definition for BFpage, structure of a buffer entry				  */
/******************************************************************************/
typedef struct BFpage {
	PFpage         	fpage;	     	/* page data from the file                 */
	struct BFpage*	nextpage;   	/* next in the linked list of buffer pages */
	struct BFpage*	prevpage;   	/* prev in the linked list of buffer pages */
	bool_t         	dirty;       	/* TRUE if page is dirty                   */
	short          	count;       	/* pin count associated with the page      */
	int            	pageNum;     	/* page number of this page                */
	int            	fd;          	/* PF file descriptor of this page         */
} BFpage;


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