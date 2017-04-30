#include "minirel.h"

/* This type should be in hf.h*/

typedef struct HFHeader{
    int RecSize;                 /* Record size */
    int RecPage;                 /* Number of records per page */
    int NumPg;                   /* Number of pages in file */
    int NumFrPgFile;             /* Number of free pages in the file */ 
} HFHeader;

typedef struct HFftab_ele{
	bool_t valid; 		/* set to TRUE when a file is open */
	int fd;				/* file descriptor */
	char fname[100];	/* file name */
	HFHeader header;	/* heap file header */
	bool_t dirty;		/* TRUE if HF header has changed */
} HFftab_ele;