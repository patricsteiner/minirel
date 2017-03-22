#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"

int main() {
	Hashtable* ht = ht_init(2);
	BFhash_entry* e = malloc(sizeof(BFhash_entry));
	BFhash_entry* f = malloc(sizeof(BFhash_entry));
	BFhash_entry* g = malloc(sizeof(BFhash_entry));
	BFhash_entry* h = malloc(sizeof(BFhash_entry));
	BFhash_entry* i = malloc(sizeof(BFhash_entry));
	e->nextentry = NULL;
	f->nextentry = NULL;
	g->nextentry = NULL;
	h->nextentry = NULL;
	e->preventry = NULL;
	f->preventry = NULL;
	g->preventry = NULL;
	h->preventry = NULL;
	e->fd = 88;
	e->pageNum = 33;
	f->fd=22;
	f->pageNum = 123;
	g->fd=555;
	g->pageNum = 666;
	h->fd=777;
	h->pageNum = 999;
	i->fd=777;
	i->pageNum = 998;
	ht_add(ht, e);
	ht_add(ht, h);
	ht_add(ht, f);
	ht_add(ht, g);
	ht_add(ht, i);

	ht_print(ht);
	BFhash_entry* ff = ht_get(ht, e);
	printf("pageNum of ff=e: %d\n", ff->pageNum);
	ht_remove(ht, h);
	ht_remove(ht, f);
	ht_remove(ht, e);
	ht_print(ht);
	ht_free(ht);
	ht_print(ht);
}

/*
 * uses x to generate hashcode.
 * universal hashing is used: h(x) = (ax+b) mod p mod m,
 * whereas a = 123, b = 87, p = 31, m = ht->size and x = fd*pageNum.
 */
int ht_hashcode(Hashtable* ht, BFhash_entry* entry) {
	return (123 * entry->fd * entry->pageNum + 87) % 31 % ht->size;
}

/*
 * initializes an empty hashtable with given size.
 */
Hashtable* ht_init(int size) {
	Hashtable* ht = malloc(sizeof(Hashtable));
	ht->size = size;
	ht->entries = malloc(sizeof(BFhash_entry*) * size); // allocate memory for all entry pointers
	int i;
	for (i = 0; i < size; i++) {
		ht->entries[i] = NULL;
	}
	return ht;
}

/*
 * Adds entry to hashtable.
 */
int ht_add(Hashtable* ht, BFhash_entry* entry) {
	int hc = ht_hashcode(ht, entry);
	BFhash_entry* e = ht->entries[hc];
	if (ht_get(ht, entry) != NULL) {
		return NULL; // entry already in hashtable
	}
	else if (e == NULL) {
		ht->entries[hc] = entry;
		entry->preventry = NULL;
		entry->nextentry = NULL;
	}
	else { // we have to iterate to last place of this bucket
		while (e->nextentry != NULL) {
			e = e->nextentry;
		}
		entry->preventry = e;
		e->nextentry = entry;
		entry->nextentry = NULL;
	}
	return 0;
}

/*
 * Removes entry from hashtable.
 */
int ht_remove(Hashtable* ht, BFhash_entry* entry) {
	int hc = ht_hashcode(ht, entry);
	BFhash_entry* e = ht_get(ht, entry);
	if (e == NULL) return -1; // not found
	BFhash_entry* prev = e->preventry;
	BFhash_entry* next = e->nextentry;
	if (prev != NULL && next != NULL) { // between two entries
		prev->nextentry = next;
		next->preventry = prev;
	} 
	else if (prev != NULL && next == NULL) { // when last entry in bucket
		prev->nextentry = NULL;
	}
	else if (prev == NULL && next != NULL) { // when first entry in bucket
		ht->entries[hc] = next;
	}
	else { // when only entry in bucket
		ht->entries[hc] = NULL;
	}
	free(e);
	return 0;
}

BFhash_entry* ht_get(Hashtable* ht, BFhash_entry* entry) {
	int hc = ht_hashcode(ht, entry);
	BFhash_entry* e = ht->entries[hc];
	if (e == NULL) {
		return NULL;
	}
	while (!(e->fd == entry->fd && e->pageNum == entry->pageNum)) {
		e = e->nextentry;
		if (e == NULL) return NULL; // entry not found
	}
	return e;
}

/*
 * Frees all the allocated memory. TODO
 */
int ht_free(Hashtable* ht) {
	int i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		BFhash_entry* tmp;
		while (e != NULL) {
			tmp = e;
			e = e->nextentry;
			free(tmp);
			printf("freed 1\n"); //TODO
		}
	}
}

void ht_print(Hashtable* ht) {
	int i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		while (e != NULL) {
			printf("entry %d: fd %d, pagenum %d\n", i, e->fd, e->pageNum);
			e = e->nextentry;
		}
	}
}