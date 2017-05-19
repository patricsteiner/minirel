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

AMitab_ele *AMitab;
AMscantab_ele *AMscantab;
size_t AMitab_length;
size_t AMscantab_length;

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

	/* fill the array of the hf file table*/ 
	if (AMitab_length >= AM_ITAB_SIZE){
		return AME_FULLTABLE;
	}
	pt = AMitab + AMitab_length;
	AMitab++;

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
	pt->racine_page = 2;
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

	memcpy((char*) headerbuf, (int*) &pt->header.indexNo, sizeof(int));
	memcpy((char*) headerbuf, (int*) &pt->header.attrType, sizeof(char));
	memcpy((char*) headerbuf, (int*) &pt->header.attrLength, sizeof(int));
	memcpy((char*) headerbuf, (int*) &pt->header.height_tree, sizeof(int));
	memcpy((char*) headerbuf, (int*) &pt->header.nb_leaf, sizeof(int));


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
	return AME_OK;
}

int AM_CloseIndex(int fileDesc){
	return AME_OK;
}

int AM_InsertEntry(int fileDesc, char *value, RECID recId){
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
 */
int AM_OpenIndexScan(int AM_fd, int op, char *value){
	AMitab_ele* amitab_ele;
	AMscantab_ele* amscantab_ele;
	
	if (AM_fd < 0 || AM_fd >= AMitab_length) return AME_FD;
	if (op < 1 || op > 6) return AME_INVALIDOP;
	if (AMscantab_length >= AM_ITAB_SIZE) return AME_SCANTABLEFULL;

	amitab_ele = AMitab + AM_fd;
	
	if (amitab_ele->valid != TRUE) return AME_INDEXNOTOPEN;
	
	amscantab_ele = AMscantab + AMscantab_length;
	
	/* copy the values */
	memcpy((char*) amscantab_ele->value, (char*) value, amitab_ele->header.attrLength);
	amscantab_ele->op = op;
	amscantab_ele->current = amitab_ele->racine_page; /* always start at the root */
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
 */
RECID AM_FindNextEntry(int scanDesc){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;

	return res;
}

/*
 *   int     scanDesc;           scan descriptor of an index
 *
 * This function terminates an index scan and disposes of the scan state information. It returns AME_OK 
 * if the scan is successfully closed, and an AM error code otherwise.
 */
int AM_CloseIndexScan(int scanDesc) {
	AMscantab_ele* amscantab_ele;

	if (scanDesc < 0 ||  (scanDesc >= AMscantab_length && AMscantab_length !=0)) return AME_INVALIDSCANDESC;

	amscantab_ele = AMscantab + scanDesc;

	if (amscantab_ele->valid == FALSE) return AME_SCANNOTOPEN; 

	/* done similar to the closeScan in HF. is this procedure not needed? */
	if (scanDesc == AMscantab_length - 1) { /* if last scan in the table */
		if (AMscantab_length > 0) AMscantab_length--;
		while( ((AMscantab + AMscantab_length - 1)->valid == FALSE) && (AMscantab_length > 0)) {
			AMscantab_length--; /* delete all following scan with valid == FALSE */
		}
	}

	return AME_OK;
}

void AM_PrintError(char *errString){
	printf("%s\n",errString);
}