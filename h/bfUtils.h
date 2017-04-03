#ifndef __BF_HEADER_H__
#define __BF_HEADER_H_

/******************************************************************************/
/*   Type definition for pages of the BF layer.                               */
/******************************************************************************/

typedef struct BFpage {
	int unixfd; 	 /* PF file descriptor*/
	PFpage fpage;   /*page data from a file*/
	struct BFpage *nextpage;/*next in the linked list of buffer pages	*/
	struct BFpage *prevpage;/* prev in the linked list of buffer pages*/
	bool_t dirty; /*TRUE if page is dirty*/
	short count; /* pin count associated with the page*/
	int pagenum;/*page number of this page*/
	int fd; /* PF file descriptor of this page*/
}BFpage;

typedef struct BFhash_entry {
  struct BFhash_entry *nextentry;     /* next hash table element or NULL */
  struct BFhash_entry *preventry;     /* prev hash table element or NULL */
  int fd;                             /* file descriptor                 */
  int pagenum;                        /* page number                     */
  struct BFpage *bpage;               /* ptr to buffer holding this page */
}BFhash_entry;

typedef struct Hashtable {
	size_t size;
	BFhash_entry** entries;
}Hashtable;

typedef struct Freelist {
	BFpage*			head;			/* first el of the Freelist */
	unsigned int 	size;			/* current size 			*/		
	unsigned int 	max_size;		/* max_size allowed 		*/
}Freelist;

typedef struct { 
	BFpage *head; 
	BFpage *tail;
	int number_of_page; /* to keep track of number of page */
}LRU;


/*
 * prototypes for Under BF-layer functions
 */
Freelist* fl_init(int max_size);
BFpage* fl_give_one(Freelist* fl);
int fl_add(Freelist* fl, BFpage* BFpage);
int fl_free(Freelist* fl);
void fl_print(Freelist* fl);

int ht_hashcode(Hashtable* ht, int fd, int pageNum);
Hashtable* ht_init(size_t size);
int ht_add(Hashtable* ht, BFpage* page);
int ht_remove(Hashtable* ht, int fd, int pageNum);
BFhash_entry* ht_get(Hashtable* ht, int fd, int pageNum);
int ht_free(Hashtable* ht);

LRU* lru_init();
int lru_add(LRU* lru, BFpage *new_BF_page);
char lru_find(LRU* lru, BFpage *page);
int lru_remove(LRU* lru, BFpage** victim);
void lru_print(LRU* lru);
int lru_free(LRU* lru);
int lru_mtu(LRU* lru, BFpage* mtu_page);

/*
 * prototypes for more BF-layer functions
 */

int BF_FlushPage(LRU* lru);


/**********************************************************************************************/
/*More errors use for BF layer                                                                */
/**********************************************************************************************/

#define BFE_UNPINNEDPAGE        (-54)
#define BFE_NOVICTIM            (-55)
#define BFE_EMPTY               (-56)
#define BFE_PAGENOTOBUF		(-57)
#define BFE_PINNEDPAGE          (-58)

#endif
