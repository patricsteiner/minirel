#include "minirel.h"

/* This type should be in hf.h*/

typedef struct HFHeader{
    int rec_size;			/* Record size */
    int rec_page;			/* Number of records per page */
    int num_pages;			/* Number of pages in file */
    int num_free_pages;		/* Number of free pagesin the file */ 
    char pageDirectory[PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char)]; /* a table that is 1 if page is full, 0 otherwise */
} HFHeader;

typedef struct HFftab_ele{
	bool_t valid; 		/* set to TRUE when a file is open */
	int fd;				/* file descriptor */
	char fname[100];	/* file name */
	HFHeader header;	/* heap file header */
	bool_t dirty;		/* TRUE if HF header has changed */
} HFftab_ele;
