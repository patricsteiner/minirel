#ifndef __AMUTILS_H__
#define __AMUTILS_H__

#define ROOT -1
#define FIRST_LEAF -1
#define LAST_LEAF -1

#include "minirel.h"

typedef struct AMHeader{
     int     indexNo;           /* id of this index for the file   */
     char    attrType;          /* 'c', 'i', or 'f'                */
     int     attrLength;        /* 4 for 'i' or 'f', 1-255 for 'c' */
     int     height_tree;    /*height of the b+tree, number of levels (if only root ==> one level)*/
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

/* couples pointer,key of an internal node */ 
typedef struct icouple{
	int pagenum; 
	int key;
	
	
}icouple; 
typedef struct fcouple{
	int pagenum; 
	float key;
	
	
}fcouple; 

typedef struct ccouple{
	int pagenum; 
	char*key; /* variable length, consequently has to be malloc */
	
	
}ccouple; 

/* couples of leaf*/
typedef struct icoupleLeaf{
	RECID recid; 
	   int key;
	
}icoupleLeaf;

typedef struct fcoupleLeaf{
	RECID recid; 
	 float  key;
	
}fcoupleLeaf;

typedef struct ccoupleLeaf{
	RECID recid; 
	 char *  key;
	
}ccoupleLeaf;
/* 3 kind of nodes because three different key types*/
/* it is important to notice that those structs are only used to get element from a buffer page where a not is written */
/* this is like a skeleton, that we fill when we need */

typedef struct inode{
	bool_t is_leaf;            /* first element checked, is the boolean a node ? */ 
	int num_keys;		/* number of key into the node*/
	int last_pt;            /* there is one more pointer than keys */
	icouple* couple;		
	
	
} inode;

typedef struct fnode{
	bool_t is_leaf;            /* first element checked, is the boolean a node ? */ 
	int num_keys;		/* number of keys written into the node at a certain */ 
	int last_pt;            /* there is one more pointer than keys */
	fcouple* couple;		/* offset= sizeof(bool_t) + sizeof(int) + key_position* sizeof(couple)*/
	
	
} fnode;

typedef struct cnode{
	bool_t is_leaf;            /* first element checked, is the boolean a node ? */ 
	int num_keys;		/* number of key into the node*/
	int last_pt;            /* there is one more pointer than keys */
	ccouple* couple;		
	
	
} cnode;

/* struct of leaves, also only use to read all informations of the pagebuf where the leaf is stored */	
typedef struct ileaf{
	bool_t is_leaf;            /* first element checked, is the boolean a node */ 
	int num_keys;		/* number of key into the node*/ 
	int previous;           /* pagenum of the previous page */
	RECID last_recid;            /* there is one more pointer than keys */
	int next; /* pagenum of the next page */
	icoupleLeaf* couple;     /* a pointer on the beginning of the array of couple */
} ileaf;

typedef struct fleaf{
	bool_t is_leaf;            /* first element checked, is the boolean a node */ 
	int num_keys;		/* number of key into the node*/ 
	int previous;           /* pagenum of the previous page */
	RECID last_recid;            /* there is one more pointer than keys */
	int next; /* pagenum of the next page */
	fcoupleLeaf* couple;     /* a pointer on the beginning of the array of couple */
} fleaf;

typedef struct cleaf{
	bool_t is_leaf;            /* first element checked, is the boolean a node */ 
	int num_keys;		/* number of key into the node*/ 
	int previous;           /* pagenum of the previous page */
	RECID last_recid;            /* there is one more pointer than keys */
	int next; /* pagenum of the next page */
	ccoupleLeaf* couple;     /* a pointer on the beginning of the array of couple */
} cleaf;

/*
Page 0 : PFHeader Page
Page 1 : AMHeader Page
On the AMHeader Page (see AMHeader struct)
Others pages :
Each page is a node (starting from page 2)

/*more errors*/
#define	AME_INDEXNOTOPEN		(-25)
#define AME_WRONGROOT                   (-26)
#define AME_ATTRTYPE			(-27) /* Invalid attribute type in file scan*/
#define AME_ATTRLENGTH			(-28) /* Invalid attribute length */
#define AME_ATTROFFSET   		(-29) /* Invalid attribute offset */
#define AME_OPERATOR			(-30) /* Invalid Operator in file scan */












#endif 
