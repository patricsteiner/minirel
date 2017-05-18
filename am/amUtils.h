#ifndef __AMUTILS_H__
#define __AMUTILS_H__

#include "minirel.h"

typedef struct AMitab_ele{
	bool_t valid; 		/* set to TRUE when a file is open */
	int fd;			/* pf file descriptor */
	char fname[255];	/* file name */
	char iname[255+4];      /* index name */
	int racine_page;        /* the page number where the racine is */
	int fanout;             /* fanout of the btree, known with the size of the key */
	AMHeader header;	/* heap file header */
	bool_t dirty;		/* TRUE if HF header has changed */
	 
} AMitab_ele;


typedef struct AMHeader{

     int     indexNo;           /* id of this index for the file   */
     char    attrType;          /* 'c', 'i', or 'f'                */
     int     attrLength;        /* 4 for 'i' or 'f', 1-255 for 'c' */

     int     height_tree;    /*height of the b+tree*/
     int     nb_leaf;        /* number of leaf */
} AMHeader;


















#endif 
