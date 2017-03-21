#include "hashtable.h"

/*
 * uses x to generate hashcode.
 * universal hashing is used: h(x) = (ax+b) mod p mod m,
 * whereas a = 123, b = 87, p = 31, m = ht->size.
 */
int ht_hashcode(Hashtable* ht, int x) {
	return (123 * x + 87) % 31 % ht->size;
}

/*
 * initializes an empty hashtable with given size.
 */
Hashtable* ht_init(int size) {
	Hashtable* ht = malloc(sizeof(Hashtable));
	ht->size = size;
	ht->entries = malloc(sizeof(BFhash_entry*) * size);
	for (int i = 0; i < size; i++) {
		ht->entries[i] = NULL;
	}
	return ht;
}

/*
 * Adds entry to hashtable.
 */
int ht_add(Hashtable* ht, BFhash_entry* entry) {
	int hc = ht_hashcode(ht, entry->fd * entry->pagenum);
	if (ht_get(ht, entry) < 0) {
		ht->entries[hc] = malloc(sizeof(BFhash_entry));
	} else {
		//iterate through list until next free
	}
}

/*
 * Removes entry from hashtable.
 */
int ht_remove(Hashtable* ht, BFhash_entry* entry) {

}

BFhash_entry* ht_get(Hashtable* ht, BFhash_entry* entry) {

}

/*
 * Frees all the allocated memory.
 */
int ht_free(Hashtable* ht) {

}