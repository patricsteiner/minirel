#ifndef __LRU_H__
#define __LRU_H__



typedef struct { 
	BFpage *head; 
	BFpage *tail;
	int number_of_page; // to keep track of number of page 
}LRU;

/*
 * Initialize an empty LRU
 */
LRU* lru_init();

/*
 * Add BFpage to the doubly linked LRU
 */
int lru_add(LRU* lru, BFpage *new_BF_page);

/*
 * Return 0 if the page is in the LRU
 */
char lru_find(LRU* lru, BFpage *page);

/*
 *  Chooses a victim in the LRU and removes a BFpage from LRU, return a pointer on this victim
 */
BFpage* LRU_remove(LRU* lru);

/*
 * Frees the allocated memory.
 */
int lru_free(LRU* lru);



#endif
	
