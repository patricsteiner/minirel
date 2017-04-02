#include <stdio.h>
#include <stdlib.h>
#include "hashtable.h"
#include "bfHeader.h"

/*int main() {
	BFhash_entry* ff;
	Hashtable* ht = ht_init(2);
	BFpage* e = malloc(sizeof(BFpage));
	BFpage* f = malloc(sizeof(BFpage));
	BFpage* g = malloc(sizeof(BFpage));
	BFpage* h = malloc(sizeof(BFpage));
	BFpage* i = malloc(sizeof(BFpage));
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
	
	ff = ht_get(ht, 88,33);
	printf("pageNum of ff=e: %d\n", ff->pageNum);
	ht_remove(ht, 22,123);
	//*ht_remove(ht, 88,33);
	ht_remove(ht, 777,998);
	ht_print(ht);
	ht_free(ht);
	ht_print(ht);
}*/

/*
 * uses x to generate hashcode.
 * universal hashing is used: h(x) = (ax+b) mod p mod m,
 * whereas a = 123, b = 87, p = 31, m = ht->size and x = fd*pageNum.
 */
int ht_hashcode(Hashtable* ht, int fd, int pageNum) {
	return (123 * fd * pageNum + 87) % 31 % ht->size;
}

/*
 * initializes an empty hashtable with given size.
 */
Hashtable* ht_init(size_t size) {
	Hashtable* ht = malloc(sizeof(Hashtable));
	ht->size = size;
	ht->entries = malloc(sizeof(BFhash_entry*) * size); /* allocate memory for all entry pointers */
	size_t i;
	for (i = 0; i < size; i++) {
		ht->entries[i] = NULL;
	}
	return ht;
}

/*
 * Adds a page to hashtable.
 * return 0
 */
int ht_add(Hashtable* ht, BFpage* page) {
	int hc = ht_hashcode(ht, page->fd, page->pageNum);
	if (ht_get(ht, page->fd, page->pageNum) != NULL) {
		return NULL; /* entry already in hashtable */
	}
	else { /* entry needs to be created and inserted */
		BFhash_entry* newEntry = malloc(sizeof(BFhash_entry));
		newEntry->fd = page->fd;
		newEntry->pageNum = page->pageNum;
		newEntry->bpage = page;
		BFhash_entry* entry = ht->entries[hc];
		if (entry == NULL) { /* bucket is empty, insert it right here */
			ht->entries[hc] = newEntry;
			newEntry->preventry = NULL;
			newEntry->nextentry = NULL;
		}
		else { /* we have to iterate to last place of this bucket */
			while (entry->nextentry != NULL) {
				entry = entry->nextentry;
			}
			newEntry->preventry = entry;
			entry->nextentry = newEntry;
			newEntry->nextentry = NULL;
		}
	}
	return 0;
}

/*
 * Removes entry from hashtable.
 */
int ht_remove(Hashtable* ht, int fd, int pageNum) {
	int hc = ht_hashcode(ht, fd, pageNum);
	BFhash_entry* e = ht_get(ht, fd, pageNum);
	if (e == NULL) return -1; /* not found */
	BFhash_entry* prev = e->preventry;
	BFhash_entry* next = e->nextentry;
	if (prev != NULL && next != NULL) { /* between two entries */
		prev->nextentry = next;
		next->preventry = prev;
	} 
	else if (prev != NULL && next == NULL) { /* when last entry in bucket */
		prev->nextentry = NULL;
	}
	else if (prev == NULL && next != NULL) { /* when first entry in bucket */
		ht->entries[hc] = next;
	}
	else { /* when only entry in bucket */
		ht->entries[hc] = NULL;
	}
	free(e);
	return 0;
}

BFhash_entry* ht_get(Hashtable* ht, int fd, int pageNum) {
	int hc = ht_hashcode(ht, fd, pageNum);
	BFhash_entry* e = ht->entries[hc];
	if (e == NULL) {
		return NULL;
	}
	while (!(e->fd == fd && e->pageNum == pageNum)) {
		e = e->nextentry;
		if (e == NULL) return NULL; /* entry not found */
	}
	return e;
}

/*
 * Frees all the allocated memory (all hashtable entries and the hashtable itself).
 */
int ht_free(Hashtable* ht) {
	size_t i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		BFhash_entry* tmp;
		while (e != NULL) {
			tmp = e;
			e = e->nextentry;
			free(tmp);
		}
		ht->entries[i] = NULL;
	}
	free(ht);
}

void ht_print(Hashtable* ht) {
	size_t i;
	for (i = 0; i < ht->size; i++) {
		BFhash_entry* e = ht->entries[i];
		while (e != NULL) {
			printf("bucket %d: fd %d, pagenum %d\n", i, e->fd, e->pageNum);
			e = e->nextentry;
		}
	}
}