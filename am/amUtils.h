#ifndef __AMUTILS_H__
#define __AMUTILS_H__

#include "minirel.h"

typedef struct AMHeader{
     int     indexNo;           /* id of this index for the file   */
     char    attrType;          /* 'c', 'i', or 'f'                */
     int     attrLength;        /* 4 for 'i' or 'f', 1-255 for 'c' */
     int     height_tree;    /*height of the b+tree*/
     int     nb_leaf;        /* number of leaf */
} AMHeader;


typedef struct AMitab_ele{
	bool_t valid; 		/* set to TRUE when a file is open */
	int fd;			/* pf file descriptor */
	char iname[255+4];      /* index name */
	int racine_page;        /* the page number where the racine is */
	int fanout;             /* max number of key per internal node (since each page is a node)*/
	AMHeader header;	/* heap file header */
	bool_t dirty;		/* TRUE if HF header has changed */
	 
} AMitab_ele;




/* Type representing a node in a B+tree.
 * Same type for leaves and internal node.
 * 
 * In a Leaf : 
 * Index of each key = index of corresponding pointer
 * Max of : order - 1 , key-pointer pair
 * Last pointer points to the leaf to the right (NULL if rightmost)
 * 
 * In an Internal Node : 
 * first pointer refers to lower node (with key less than the smallest key
 * in the keys array.)
 */
typedef struct Node{
	bool_t is_leaf;
	int parent;			/* pagenum of parent node , 0 if the root */
	int num_keys;		/* number of key into the node*/
	int prev_leaf;		/* pagenum of previous leaf, 0 if first leaf */
	int next_leaf;		/* pagenum of next leaf, 0 if last leaf */
} Node;

/*
Page 0 : PFHeader Page
Page 1 : AMHeader Page
On the AMHeader Page (see AMHeader struct)
Others pages :
Each page is a node (starting from page 2)

/*more errors*/
#define	AME_INDEXNOTOPEN		(-25)












#endif 
