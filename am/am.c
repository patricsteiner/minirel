#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "minirel.h"
#include "pf.h"
#include "../pf/pfUtils.h"
#include "hf.h"
#include "../hf/hfUtils.h"
#include "am.h"
#include "../am/amUtils.h"


/* B tree*/
/* policy to go through the B tree using a particular value, if the value if len than a key in a node or a leaf then the previous pointer has to be taken, otherwise ( greater or equal) the value is check with the next keys in the node until it is less than one or until the last key (the last pointer of the node is taken then )*/
/* int AM_FindLeaf(int ifd,char* value, int* tab) algorithm: -check parameter: the size of tab is the height of the tree and the first element is the page num of root.
							 - go through the Btree with the policy explain above, and stock the pagenum of every scanned node in the array tab , at the end the pagenum of the leaf where the value has to be insert is returned 
							 - return error or HFE_OK, in case of duplicate key: the position of the first equal key in the leaf*/
AMitab_ele *AMitab;
AMscantab_ele *AMscantab;
size_t AMitab_length;
size_t AMscantab_length;

bool_t compareNumber(int a, int b, int op) {
	switch (op) {
		case EQ_OP: return a == b ? TRUE : FALSE;
		case LT_OP: return a < b ? TRUE : FALSE;
		case GT_OP: return a > b ? TRUE : FALSE;
		case LE_OP: return a <= b ? TRUE : FALSE;
		case GE_OP: return a >= b ? TRUE : FALSE;
		case NE_OP: return a != b ? TRUE : FALSE;
		default: return FALSE;
	}
}

bool_t compareChars(char* a, char* b, int op, int len) {
	switch (op) {
		case EQ_OP: return strncmp(a, b, len) == 0 ? TRUE : FALSE;
		case LT_OP: return strncmp(a, b, len) < 0 ? TRUE : FALSE;
		case GT_OP: return strncmp(a, b, len) > 0 ? TRUE : FALSE;
		case LE_OP: return strncmp(a, b, len) <= 0 ? TRUE : FALSE;
		case GE_OP: return strncmp(a, b, len) >= 0 ? TRUE : FALSE;
		case NE_OP: return strncmp(a, b, len) != 0 ? TRUE : FALSE;
		default: return FALSE;
	}
}

/* For any type of node: given a value and the position of a couple ( pointer,key) in a node, return the pointer if it has to be taken or not by comparing key and value. Return 0 if the pointer is unknown.
 * the fanout is also given, in case of the couple contain the last key, then the last pointer of the node has to be taken
 * this function is created to avoid switch-case inside a loop
 */
bool_t AM_CheckPointer(int pos, int fanout, char* value, char attrType, int attrLength, char* pagebuf){
	/*use to get any type of node and leaf from a buffer page*/
	inode inod;
	fnode fnod;
	cnode cnod;
	/*ileaf* ilcast;
	fleaf* flcast;
	cleaf* clcast;*/
	
	switch(attrType){
		case 'i': 
			  /* fill the struct using offset and cast operations */
			  inod.num_keys= *((int*)pagebuf+sizeof(bool_t));
			  inod.couple=(icouple *)pagebuf+sizeof(bool_t)+sizeof(int);
			  inod.last_pt=*((int*)(pagebuf+sizeof(bool_t)+(inod.num_keys)*2*sizeof(int)+sizeof(int)));
			  
			  if( pos<fanout-1){ /* fanout -1 is the number of couple (pointer, key)*/
				if( strncmp((char*) &(inod.couple[pos].key), (char*) value,  sizeof(int)) <0) return inod.couple[pos].pagenum;
				return 0;
			   }
			   else if( pos=fanout-1) {
			   	if( strncmp((char*) &(inod.couple[pos].key), (char*) value,  sizeof(int)) >=0) return inod.last_pt;
				return 0;
			   }
			   else {
				printf( "DEBUG: pb with fanout or position given \n");
				return -1;
			   }
			   break;

		case 'c':  /* fill the struct using offset and cast operations */
			  cnod.num_keys= *((int*)pagebuf+sizeof(bool_t));
			  cnod.couple=(ccouple*)pagebuf+sizeof(bool_t)+sizeof(int);
			  cnod.last_pt=*((int*)(pagebuf+sizeof(bool_t)+(cnod.num_keys)*2*sizeof(int)+sizeof(int)));
			  if( pos<fanout-1){ /* fanout -1 is the number of couple (pointer, key)*/
				if( strncmp((char*) cnod.couple[pos].key, (char*) value,  attrLength) <0) return cnod.couple[pos].pagenum;
				return 0;
			   }
			   else if( pos=fanout-1) {
			   	if( strncmp((char*) cnod.couple[pos].key, (char*) value,  attrLength) >=0) return cnod.last_pt;
				return 0;
			   }
			   else {
				printf( "DEBUG: pb with fanout or position given \n");
				return -1;
			   }
			   break;

		case 'f':  /* fill the struct using offset and cast operations */
			  fnod.num_keys= *((int*)pagebuf+sizeof(bool_t));
			  fnod.couple=(fcouple*)pagebuf+sizeof(bool_t)+sizeof(int);
			  fnod.last_pt=*((int*)(pagebuf+sizeof(bool_t)+(fnod.num_keys)*2*sizeof(int)+sizeof(int)));

			  if( pos<(fanout-1)){ /* fanout -1 is the number of couple (pointer, key)*/
				if(  fnod.couple[pos].key > *((float*)value) ) return fnod.couple[pos].pagenum;
				return 0;
			   }
			   else if(( pos==fanout-1)) {
			   	if(  fnod.couple[pos].key <=*((float*)value) ) return fnod.last_pt;
				return 0;
			   }
			   else {
				printf( "DEBUG: pb with fanout or position given \n");
				return -1;
			   }
			   break;
		default:
			
			return AME_ATTRTYPE;
	}
}

	
	 
int AM_FindLeaf(int idesc, char* value, int* tab){
	int error,  pagenum;
	bool_t is_leaf;
	AMitab_ele* pt;
	int fanout;
	char* pagebuf;
	int i,j;
	
	
	/* all verifications of idesc and root number has already been done by the caller function */
	pt=AMitab+idesc;
	
	if(tab[0]<=1) return AME_WRONGROOT;
	
	for(i=0; i<(pt->header.height_tree);i++){

		error=PF_GetThisPage(pt->fd, tab[i], &pagebuf);
		if(error!=PFE_OK) PF_ErrorHandler(error);
		
		/*check if this node is a leaf */
		memcpy((bool_t*) is_leaf, pagebuf, sizeof(bool_t));
		
		if( is_leaf==FALSE){
				fanout=pt->fanout;
			/* have to find the next child using comparison */
			/* fanout = n ==> n-1 key */
				for(j=0; j<(pt->fanout-1);j++){
					}
		}
   

	}
					
					
						
			
}





/* 
 *
 */
void AM_Init(void){
	AMitab = malloc(sizeof(AMitab_ele) * AM_ITAB_SIZE);
	AMitab_length = 0;
	HF_Init();
}



int AM_CreateIndex(char *fileName, int indexNo, char attrType, int attrLength, bool_t isUnique){
	int error, fd, pagenum;
	AMitab_ele* pt;
	char* headerbuf;
/**********************add verif on all attribute check HF_OpenScan */
	/* fill the array of the hf file table*/ 
	if (AMitab_length >= AM_ITAB_SIZE){
		return AME_FULLTABLE;
	}
	pt = AMitab + AMitab_length;
	AMitab_length++;

	sprintf(pt->iname, "%s.%d", fileName, indexNo);

	error = PF_CreateFile(pt->iname);
	if(error != PFE_OK)
		PF_ErrorHandler(error);

	fd = PF_OpenFile(pt->iname);
	if(fd < 0 )
		return AME_PF;

	pt->fd = fd;
	pt->valid = TRUE;
	pt->dirty = TRUE;
	pt->racine_page = -1;
	/* Since on a internal node, there  is 3 int(is_leaf, pagenum of parent, number of key*/ 
	pt->fanout = ( (PF_PAGE_SIZE ) - (3+1)*sizeof(int)) / (sizeof(int) + attrLength);
	pt->header.indexNo = indexNo;
	pt->header.attrType = attrType;
	pt->header.attrLength = attrLength;
	pt->header.height_tree = 1;
	pt->header.nb_leaf = 1;

	error = PF_AllocPage(fd, &pagenum, &headerbuf);
	if (error != PFE_OK)
		PF_ErrorHandler(error);

	if (pagenum != 1)
		return AME_PF;

	memcpy((char*) (headerbuf), (int*) &pt->header.indexNo, sizeof(int));
	memcpy((char*) (headerbuf + sizeof(int)), (int*) &pt->header.attrType, sizeof(char));
	memcpy((char*) (headerbuf + sizeof(int) + sizeof(char)), (int*) &pt->header.attrLength, sizeof(int));
	memcpy((char*) (headerbuf + 2*sizeof(int) + sizeof(char)), (int*) &pt->header.height_tree, sizeof(int));
	memcpy((char*) (headerbuf + 3*sizeof(int) + sizeof(char)), (int*) &pt->header.nb_leaf, sizeof(int));


	pt->valid = FALSE;
	
	error = PF_UnpinPage(pt->fd, pagenum, 1);
	if (error != PFE_OK)
		return PF_ErrorHandler(error);

	error = PF_CloseFile(pt->fd);
	if (error != PFE_OK)
		PF_ErrorHandler(error);


	AMitab_length--;
	printf("Index : %s created\n", pt->iname);
	return AME_OK;
}

int AM_DestroyIndex(char *fileName, int indexNo){
	int error;
	char* new_filename;

	new_filename = malloc(sizeof(fileName) + sizeof(int));
	sprintf(new_filename, "%s.%i", fileName, indexNo);
	error = PF_DestroyFile(new_filename);
	if (error != PFE_OK)
		PF_ErrorHandler(error);

	printf("Index : %s has been destroyed\n", new_filename);
	free(new_filename);
	return AME_OK;
}

int AM_OpenIndex(char *fileName, int indexNo){
	int error, pffd, fileDesc;
	AMitab_ele *pt;
	char *headerbuf;
	char* new_filename;
	
	/*Initialisation */
	new_filename = malloc(sizeof(fileName) + sizeof(int));
	printf("length %d", AMitab_length);
	/*parameters cheking */
	if( AMitab_length >= AM_ITAB_SIZE){
		return AME_FULLTABLE;
	}
	sprintf(new_filename, "%s.%i", fileName, indexNo);
	pffd = PF_OpenFile(new_filename);
	if(pffd < 0){
		PF_ErrorHandler(pffd);
	}

	/* read the header which are stored on the second page of the file (index = 1) */ 
	error = PF_GetThisPage(pffd, 1, &headerbuf);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}


	/* Fill the array of the AM index table */
	
	pt = AMitab + AMitab_length;
	
	pt->fd = pffd;
	pt->valid = TRUE;
	pt->dirty = FALSE;
	pt->racine_page = 2;
	
	memcpy(pt->iname, new_filename, sizeof(new_filename));
	free(new_filename);
	memcpy( (int*) &pt->header.indexNo,(char*) (headerbuf), sizeof(int));
	memcpy((int*) &pt->header.attrType,(char*) (headerbuf + sizeof(int)),  sizeof(char));
	memcpy( (int*) &pt->header.attrLength, (char*) (headerbuf + sizeof(int) + sizeof(char)),sizeof(int));
	memcpy((int*) &pt->header.height_tree,(char*) (headerbuf + 2*sizeof(int) + sizeof(char)),  sizeof(int));
	memcpy( (int*) &pt->header.nb_leaf,(char*) (headerbuf + 3*sizeof(int) + sizeof(char)), sizeof(int));

	pt->fanout = ( (PF_PAGE_SIZE ) - (3+1)*sizeof(int)) / (sizeof(int) + pt->header.attrLength);



	/*increment the size of the table*/
	AMitab_length ++;
	fileDesc = AMitab_length -1 ;
	/*unpin and touch the header page */
	error = PF_UnpinPage(pt->fd, 1, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	
	return fileDesc;
	
}

int AM_CloseIndex(int fileDesc){

	AMitab_ele* pt;
	int error;
	char* pagebuf;
	int i;
	i=0;
	
	

	if (fileDesc < 0 || fileDesc >= AMitab_length) return AME_FD;
	pt=AMitab + fileDesc ;
	/*check is file is not already close */
	if (pt->valid!=TRUE) return AME_INDEXNOTOPEN;


	/* check the header */
	if (pt->dirty==TRUE){ /* header has to be written again */
		error=PF_GetThisPage( pt->fd, 1, &pagebuf);
		if(error!=PFE_OK)PF_ErrorHandler(error);


		/* write the header  */
		memcpy((char*) (headerbuf), (int*) &pt->header.indexNo, sizeof(int));
		memcpy((char*) (headerbuf + sizeof(int)), (int*) &pt->header.attrType, sizeof(char));
		memcpy((char*) (headerbuf + sizeof(int) + sizeof(char)), (int*) &pt->header.attrLength, sizeof(int));
		memcpy((char*) (headerbuf + 2*sizeof(int) + sizeof(char)), (int*) &pt->header.height_tree, sizeof(int));
		memcpy((char*) (headerbuf + 3*sizeof(int) + sizeof(char)), (int*) &pt->header.nb_leaf, sizeof(int));
		
		/* the page is dirty now ==> last arg of PF_UnpinPage ("dirty") set to one */
		error = PF_UnpinPage(pt->fd, 1, 1);
		if(error != PFE_OK) PF_ErrorHandler(error);
		
		
	}
	/* close the file using pf layer */
	error=PF_CloseFile(pt->fd);
	if(error!=PFE_OK)PF_ErrorHandler(error);

	/* check that there is no scan in progress involving this file*/
	/* by scanning the scan table */
	/*****************for (i=0;i<AMscantab_length;i++){
			if((AMcantab+i)->AMfd==fileDesc) return AMESCANOPEN;
	}
	*/

	
	/*deletion */
	/* a file can be deleted only if it is at then end of the table in order to have static file descriptor */
	/* In any case the boolean valid is set to false to precise that this file is closed */
        pt->valid==FALSE;
	if(fileDesc==(AMitab_length-1)){ /* it is the last file of the table */ 
		AMitab_length--;

		if(AMitab_length >0){
			AMitab_length--;
			while (AMitab_length>0 &&  ((AMitab+ AMitab_length-1)->valid==FALSE)){
				AMitab_length--; /* delete all the closed file, which are the end of the table */
			}
		}
	}
	

	return AME_OK;
}



/* return AME_OK if succeed
 *
 */
int AM_InsertEntry(int fileDesc, char *value, RECID recId){
	int leafNum;
	int root_pagenum;
	int valuePos;
	int attrType;
	int num_keys;
	int error;
	int offset;
	char attrType;
	int* visitedNode; /* array of node visited : [root, level1, , ] */
	char* pagebuf;

	pt = AMitab + fileDesc;

	/* CASE : NO ROOT */
	if (pt->racine_page == -1){
		/* Create a tree */
		error = PF_AllocPage(pt->fd, &root_pagenum, pagebuf);
		if (error != PFE_OK)
			PF_ErrorHandler(error);

		/* Write the node into the buffer: what kind of node */
		offset = 0;
		memcpy((char*)(headerbuf + offset),(bool_t *) TRUE, sizeof(bool_t)); /* is leaf */
		offset += sizeof(bool_t);
		memcpy((char*)(headerbuf + offset),(int *) 1, sizeof(int)); /* Number of key*/
		offset += sizeof(int);
		memcpy((char*)(headerbuf + offset ),(int *) FIRST_LEAF, sizeof(int)); /* previous leaf pagenum */
		/* sizeof( recid) : space for LAST RECID */
		offset += sizeof(int);
		offset += sizeof(RECID);
		memcpy((char*)(headerbuf offset),(int *) LAST_LEAF, sizeof(int)); /* next leaf pagenum */
		offset += sizeof(int);
		memcpy((char*)(headerbuf + offset), (RECID*) recId, sizeof(RECID));
		offset += sizeof(RECID);
		
		switch (fd->header.attrType){
			case 'c':
				memcpy((char*)(headerbuf + offset), (char*) value, fd->header.attrLength);
				break;
			case 'i':
				memcpy((char*)(headerbuf + offset), (int*) value, fd->header.attrLength);
				break;
			case 'f':
				memcpy((char*)(headerbuf + offset), (float*) value, fd->header.attrLength);
				break;
			default:
				return AME_INVALIDATTRYPE;
				break;

		}

		/* change the AMi_elem */
		pt->racine_page = 2:
		pt->header.height_tree = 1;
		pt->header.nb_leaf = 1;
		pt->dirty = TRUE;

		/* Unpin page */
		error = PF_UnpinPage(pt->fd, root_pagenum, 1);
		if (error != PFE_OK)
			PF_ErrorHandler(error);

		return AME_OK
	}


	/* FIND LEAF */

	visitedNode = malloc( pt->height * sizeof(int));
	
	/* valuePos < 0 : error,  valuePos >= 0 : position where to insert on the page */
	valuePos= AM_FindLeaf(fileDesc, value, visitedNode);
	if(valuePos < 0):
		return AME_KEYNOTFOUND;

	leafNum = visitedNode[pt->height];
	error = PF_GetThisPage(pt->fd, leafNum, &pagebuf);
	if (error != PFE_OK)
		PF_ErrorHandler(error);

	memcpy((int*) num_keys, (char*)(pagebuf + sizeof(bool_t)), sizeof(int));

	/* CASE : STILL SOME PLACE */
	if( num_keys < fanout){
		/* Insert into leaf without splitting */
		
	}
	/* CASE : NO MORE PLACE */
	else{
		/* Insert into leaf after splitting */
	}



	return AME_OK;
}

int AM_DeleteEntry(int fileDesc, char *value, RECID recId){
	return AME_OK;
}


/*
 *    int     AM_fd,               file descriptor                 
 *    int     op,                  operator for comparison         
 *    char    *value               value for comparison (or null)  
 *
 * This function opens an index scan over the index represented by the file associated with AM_fd.
 * The value parameter will point to a (binary) value that the indexed attribute values are to be
 * compared with. The scan will return the record ids of those records whose indexed attribute 
 * value matches the value parameter in the desired way. If value is a null pointer, then a scan 
 * of the entire index is desired. The scan descriptor returned is an index into the index scan 
 * table (similar to the one used to implement file scans in the HF layer). If the index scan table
 * is full, an AM error code is returned in place of a scan descriptor.
 *
 * The op parameter can assume the following values (as defined in the minirel.h file provided).
 *
 *    #define EQ_OP           1
 *    #define LT_OP           2
 *    #define GT_OP           3
 *    #define LE_OP           4
 *    #define GE_OP           5
 *    #define NE_OP           6
 *
 * Dev: Patric
 */
int AM_OpenIndexScan(int AM_fd, int op, char *value){
	AMitab_ele* amitab_ele;
	AMscantab_ele* amscantab_ele;
	int key, error, tab, val;
	val = -2147483648; /* = INTEGER_MINVALUE, used if op is NE_OP, to just get leftmost element */
	
	if (AM_fd < 0 || AM_fd >= AMitab_length) return AME_FD;
	if (op < 1 || op > 6) return AME_INVALIDOP;
	if (AMscantab_length >= AM_ITAB_SIZE) return AME_SCANTABLEFULL;

	amitab_ele = AMitab + AM_fd;
	
	if (amitab_ele->valid != TRUE) return AME_INDEXNOTOPEN;
	
	amscantab_ele = AMscantab + AMscantab_length;
	
	/* copy the values */
	memcpy((char*) amscantab_ele->value, (char*) value, amitab_ele->header.attrLength);
	amscantab_ele->op = op;


	key = AM_FindLeaf(AM_fd, value, &tab);
	if (op == NE_OP) key = AM_FindLeaf(AM_fd, (char*) &val, &tab); /* use val if NE_OP, to get leftmost leaf */

	amscantab_ele->current_page = tab[AMitab_ele->header.height_tree];
	amscantab_ele->current_key = key;
	amscantab_ele->current_num_keys = 0; /* this is set in findNextEntry */
	amscantab_ele->AMfd = AM_fd;
	amscantab_ele->valid = TRUE;
	
	return AMscantab_length++;
}

     

/* 
 * int     scanDesc;           scan descriptor of an index
 *
 * This function returns the record id of the next record that satisfies the conditions specified for an
 * index scan associated with scanDesc. If there are no more records satisfying the scan predicate, then an
 * invalid RECID is returned and the global variable AMerrno is set to AME_EOF. Other types of errors are
 * returned in the same way.
 *
 * Dev: Patric
 */
RECID AM_FindNextEntry(int scanDesc) {
	/* Procedure:
		- check operator and go to according direction (left or right) by doing:
			- increment/decrement key_pos while key_pos > 0 resp. < num_keys
			- if first/last couple is reached, jump to next/prev page
			- check on every key if its a match, if yes return and save current pos
	*/

	RECID recid;
	AMitab_ele* amitab_ele;
	AMscantab_ele* amscantab_ele;
	char* pagebuf;
	int error, current_key, current_page, direction, tmp_page, tmp_key, offset;
	float f;
	int i;
	char c[255];
	ileaf ileaf;
	fleaf fleaf;
	cleaf cleaf;
	bool_t found;

	found = FALSE;

	amitab_ele = AMitab + AM_fd;
	if (amitab_ele->valid != TRUE) return AME_INDEXNOTOPEN;
	if (scanDesc < 0 ||  (scanDesc >= AMscantab_length && AMscantab_length !=0)) return AME_INVALIDSCANDESC;
	amscantab_ele = AMscantab + scanDesc;
	if (amscantab_ele->valid == FALSE) return AME_SCANNOTOPEN; 

	/* read the current node */
	error = PF_GetThisPage(amitab_ele->fd, amscantab_ele->current, &pagebuf);
	if(error != PFE_OK) PF_ErrorHandler(error);

	/* read num_keys and write it in scantab_ele */
	memcpy((int*) &(amscantab_ele->current_num_keys), (int*) pagebuf + sizeof(bool_t), sizeof(int));

//	switch (amitab_ele->header.attrType) {
//		case 'i':
//			ileaf.previous = pagebuf + sizeof(bool_t) + sizeof(int); /* or sth, fill all fields */
//			//memcpy((ileaf*) &ileaf, (char*) (pagebuf), sizeof(inode));
//			break;
//		case 'f':
//			break;
//		case 'c':
//			break;
//	}
	
	/* iterate to right-to-left if less-operation */
	direction = (amscantab_ele->op == LT_OP || amscantab_ele->op == LE_OP) ? -1 : 1;
	
//	switch (op) {
//		case EQ_OP: /* fallthrough */
//		case NE_OP: /* fallthrough */
//		case GT_OP: /* fallthrough */
//		case GE_OP: direction = 1; break;
//		case LT_OP: /* fallthrough */
//		case LE_OP: direction = -1; break;
//	}

	/* while there is a next page (if there is non, it is set to LAST_PAGE resp. FIRST_PAGE = -1) */
	while (amscantab_ele->current_page >= 0) {
		/* iterate through keys while there is a next key and we found no result */
		while (found == FALSE && amscantab_ele->current_key > 0 && amscantab_ele->current_key < amscantab_ele->current_num_keys) {
			/* compare and return if match */
			switch (amitab_ele->header.attrType) {
			case 'i': /* fallthrough */
			case 'f': 
				if (compareNumber(, amscantab_ele->op) == TRUE) found = TRUE;
				break;
			case 'c':
				if (compareChars(, amscantab_ele->op) == TRUE) found = TRUE;
				break;
			}
			amscantab_ele->current_key += direction;
		}

		tmp_page = amscantab_ele->current_page;
		/* update amscantab_ele by reading next/prev page. the next/prev page will be -1 if its nonexistent. */
		if (direction < 0) {
			memcpy(&(amscantab_ele->current_page), (int*) (pagebuf + sizeof(bool_t) + sizeof(int)), sizeof(int));
		} else {
			memcpy(&(amscantab_ele->current_page), (int*) (pagebuf + sizeof(bool_t) + sizeof(int)*2), sizeof(int));
		}
		
		error = PF_UnpinPage(amitab_ele->fd, tmp_page, 0);
		if(error != PFE_OK) PF_ErrorHandler(error);
		error = PF_GetThisPage(amitab_ele->fd, amscantab_ele->current_page, &pagebuf);
		if(error != PFE_OK) PF_ErrorHandler(error);
		
		if (direction < 0) {
			/* read num_keys and write it in scantab_ele */
			memcpy((int*) &(amscantab_ele->current_key), (int*) pagebuf + sizeof(bool_t), sizeof(int));
		} else {
			/* if iterating right-to-left, first key on next page will just be 0 */
			amscantab_ele->current_key = 0;
		}

		/* return here, so the update and cleanup above is done in every case */
		if (found == TRUE) {
			recid.pagenum = /* TODO */
			recid.recnum = /* TODO */
			return recid;
		}	
	}






//
//	/* go down the tree until a leaf is reached */
//	while (!node.is_leaf) {
//		/* key = 0;
//		/* in every node that is visited, compare with each key until match is found 
//		while (key++ < node.num_keys) {
//			offset = sizeof(Node) + key * (amitab_ele->header.attrLength + sizeof(int));
//			/* read the pagenumber (node) 
//			memcpy((page*) &page, (char*) (pagebuf + offset) + amitab_ele->header.attrLength, sizeof(int));
//			if (amitab_ele->header.attrType == 'i') {
//				/* read the key (attr) 
//				memcpy((int*) &i, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareInt(i, amscantab_ele->value, amscantab_ele->op);
//			}
//			else if (amitab_ele->header.attrType == 'f') {
//				memcpy((float*) &f, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareFloat(f, amscantab_ele->value, amscantab_ele->op);
//			}
//			else if (amitab_ele->header.attrType == 'c') {
//				memcpy((float*) &c, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareChars(c, amscantab_ele->value, amscantab_ele->op, amitab_ele->header.attrLength);
//			}
//			if (cmp == 0) {
				//
//			}			
//		}*/
//
//		/* update the current node and read it */
//		memcpy((page*) &page, (char*) (pagebuf + sizeof(Node) + amitab_ele->header.attrLength), sizeof(int));
//		amscantab_ele->current = page;
//		error = PF_GetThisPage(amitab_ele->fd, amscantab_ele->current, &pagebuf);
//		if(error != PFE_OK) PF_ErrorHandler(error);	
//		memcpy((Node*) &node, (char*) (pagebuf), sizeof(Node));
//	}
//
//	while (node.next_leaf != 0) {
//		key = 0;
//		/* in every node that is visited, compare with each key until match is found */
//		while (key++ < node.num_keys) {
//			offset = sizeof(Node) + key * (amitab_ele->header.attrLength + sizeof(int));
//			/* read the pagenumber (node) */
//			memcpy((page*) &page, (char*) (pagebuf + offset) + amitab_ele->header.attrLength, sizeof(int));
//			if (amitab_ele->header.attrType == 'i') {
//				/* read the key (attr) */
//				memcpy((int*) &i, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareInt(i, amscantab_ele->value, amscantab_ele->op);
//			}
//			else if (amitab_ele->header.attrType == 'f') {
//				memcpy((float*) &f, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareFloat(f, amscantab_ele->value, amscantab_ele->op);
//			}
//			else if (amitab_ele->header.attrType == 'c') {
//				memcpy((float*) &c, (char*) (pagebuf + offset), amitab_ele->header.attrLength);
//				cmp = compareChars(c, amscantab_ele->value, amscantab_ele->op, amitab_ele->header.attrLength);
//			}
//			if (cmp == 0) {
//				/* it's a match, return the recid! */
//				recid.recnum = -150;
//				recid.pagenum = -150;
//				return recid;
//			}		
//		}
//	}

	error = PF_UnpinPage(amitab_ele->fd, amscantab_ele->current, 0);
	if(error != PFE_OK) PF_ErrorHandler(error);

	recid.recnum = AME_EOF;
	recid.pagenum = AME_EOF;

	return recid;
}

/*
 *   int     scanDesc;           scan descriptor of an index
 *
 * This function terminates an index scan and disposes of the scan state information. It returns AME_OK 
 * if the scan is successfully closed, and an AM error code otherwise.
 *
 * Dev: Patric
 */
int AM_CloseIndexScan(int scanDesc) {
	AMscantab_ele* amscantab_ele;

	if (scanDesc < 0 ||  (scanDesc >= AMscantab_length && AMscantab_length !=0)) return AME_INVALIDSCANDESC;

	amscantab_ele = AMscantab + scanDesc;

	if (amscantab_ele->valid == FALSE) return AME_SCANNOTOPEN; 

	/* done similar to the closeScan in HF. is this procedure not needed? */
	if (scanDesc == AMscantab_length - 1) { /* if last scan in the table */
		if (AMscantab_length > 0) AMscantab_length--;
		while( (AMscantab_length > 0) && ((AMscantab + AMscantab_length - 1)->valid == FALSE)) {
			AMscantab_length--; /* delete all following scan with valid == FALSE */
		}
	}

	return AME_OK;
}

void AM_PrintError(char *errString){
	printf("%s\n",errString);
}
