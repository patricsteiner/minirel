#include "minirel.h"
#include "bfHeader.h"

typedef struct BFhash_entry {
  struct BFhash_entry *nextentry;     /* next hash table element or NULL */
  struct BFhash_entry *preventry;     /* prev hash table element or NULL */
  int fd;                             /* file descriptor                 */
  int pageNum;                        /* page number                     */
  struct BFpage *bpage;               /* ptr to buffer holding this page */
} BFhash_entry;

typedef struct Hashtable {
	size_t size;
	BFhash_entry** entries;
} Hashtable;

/*
 * Uses entry to generate hashcode.
 */
int ht_hashcode(Hashtable* ht, int fd, int pageNum);

/*
 * Initializes an empty hashtable with given size.
 */
Hashtable* ht_init(size_t size);

/*
 * Adds entry to hashtable.
 */
int ht_add(Hashtable* ht, BFpage* page);

/*
 * Removes entry from hashtable.
 */
int ht_remove(Hashtable* ht, int fd, int pageNum);

/*
 * Retrieves the entry with given hashcode.
 * Return NULL if no entry
 */
BFhash_entry* ht_get(Hashtable* ht, int fd, int pageNum);

/*
 * Frees all the allocated memory.
 */
int ht_free(Hashtable* ht);

