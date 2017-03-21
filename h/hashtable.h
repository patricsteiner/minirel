#include "minrel.h"

typedef struct BFhash_entry {
  struct BFhash_entry *nextentry;     /* next hash table element or NULL */
  struct BFhash_entry *preventry;     /* prev hash table element or NULL */
  int fd;                             /* file descriptor                 */
  int pageNum;                        /* page number                     */
  struct BFpage *bpage;               /* ptr to buffer holding this page */
} BFhash_entry;

typedef struct Hashtable {
	unsigned int size;
	BFhash_entry** entries;
} Hashtable;

/*
 * Uses x to generate hashcode.
 */
int ht_hashcode(Hashtable* ht, x);

/*
 * Initializes an empty hashtable with given size.
 */
int ht_init(Hashtable* ht, int size);

/*
 * Adds entry to hashtable.
 */
int ht_add(Hashtable* ht, BFhash_entry* entry);

/*
 * Removes entry from hashtable.
 */
int ht_remove(Hashtable* ht, BFhash_entry* entry);

/*
 * Retrieves the entry with given hashcode.
 */
BFhash_entry* ht_get(Hashtable* ht, BFhash_entry* entry);

/*
 * Frees all the allocated memory.
 */
int ht_free(Hashtable* ht);

