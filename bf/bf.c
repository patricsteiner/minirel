#include <stdio.h>
#include "bf.h"
#include "minirel.h"


typedef struct BFpage {
  PFpage         fpage;       /* page data from the file                 */
  struct BFpage  *nextpage;   /* next in the linked list of buffer pages */
  struct BFpage  *prevpage;   /* prev in the linked list of buffer pages */
  bool_t         dirty;       /* TRUE if page is dirty                   */
  short          count;       /* pin count associated with the page      */
  int            pageNum;     /* page number of this page                */
  int            fd;          /* PF file descriptor of this page         */
} BFpage;


typedef struct BFhash_entry {
  struct BFhash_entry *nextentry;     /* next hash table element or NULL */
  struct BFhash_entry *preventry;     /* prev hash table element or NULL */
  int fd;                             /* file descriptor                 */
  int pageNum;                        /* page number                     */
  struct BFpage *bpage;               /* ptr to buffer holding this page */
} BFhash_entry;



/*
*init the BF layer : 
***create the buffer entries (empty at the beginning ) 
***add them to the free list
***init the hashtable
*/
void BF_Init(void){
	int i;

	printf("\n******** creating the empty buffer ***********\n");
	for(i=0;i<BF_MAX_BUFS;i++){
		
	}
}