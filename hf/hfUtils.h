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
	int fd;			/* pf file descriptor */
	char fname[255];	/* file name */
	HFHeader header;	/* heap file header */
	bool_t dirty;		/* TRUE if HF header has changed */
} HFftab_ele;

typedef struct HFscantab_ele{
	bool_t valid; 		    /* set to TRUE when file is being scanned*/
	int     HFfd;              /* HF file descriptor */
	RECID   current;           /* last record scanned */
	char    attrType;          /* 'c', 'i', or 'f' */
	int     attrLength;        /* 4 for 'i' or 'f', 1-255 for 'c' */
	int     attrOffset;        /* offset of attribute for comparison */
	int     op;                /* operator for comparison*/
        char    value[255];            /* value for comparison (or null) , size max == 255*/
} HFscantab_ele;
