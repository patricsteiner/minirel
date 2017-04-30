#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "minirel.h"
#include "../pf/pfUtils.h"
#include "pf.h"
#include "hf.h"
#include "../hf/hfUtils.h"

HFftab_ele *HFftab;
size_t HFftab_length;

/*
 * The number of records per page is N = (page_size / record_size) - bitmap_size
 * Bitmap : N | 0 | 0 | 1 | ..... | 0
 * Bitmap_size : (N + 1)*sizeof(int)
 * 0 : space available
 * 1 : full
 * N : Number of records on the page => int( (PAGE_SIZE/RecSize - sizeof(int)) / (1 + sizeof(int) ) )
 * 
 */


/*
 * Init the HF table structure and the layers below
 * The HF table will hold the file header data of opened files 
	int RecSize;
    int RecPage;
    int NumPg;
    int NumFrPgFile;
 * dev : Antoine
 */
void HF_Init(void){
	HFftab = malloc(sizeof(HFftab_ele)*HF_FTAB_SIZE); 
	HFftab_length = 0;
	PF_Init();
}

/*
 * This function calls PF_CreateFile() and PF_OpenFile() 
 * to create and open a paged file called fileName whose records are of size recSize. 
 * In addition, it initializes the file, by allocating a page for the header 
 * and storing appropriate information in the header page. 
 * This paged file must be closed by PF_CloseFile(). 
 * It returns HFE_OK if the new file is successfully created, and an HF error code otherwise.
 * dev : Antoine
 */
int HF_CreateFile(char *filename, int RecSize){
	int error, fd;
	HFftab_ele *pt;
	int *pagenum; /*should be equal to 1 */
	char *headerbuf;

	if(RecSize<=0){
		return HFE_RECSIZE;
	}

	error = PF_CreateFile(filename);
	if(error != PFE_OK){
		return HFE_PF;
	}
	printf("Create file\n");

	fd = PF_OpenFile(filename);
	if(fd != PFE_OK){
		return HFE_PF;
	}
	printf("Open file\n");

	/* fill the array of the hf file table*/ 
	pt = HFftab + sizeof(HFftab_ele)*HFftab_length;
	HFftab_length ++;

	pt->fd = fd;
	pt->valid = TRUE;
	sprintf(pt->fname, "%s", filename);
	pt->header.RecSize = RecSize;
	pt->header.RecPage = (int) ((PAGE_SIZE/RecSize - sizeof(int)) / (1 + sizeof(int))); /* number of recs per page */
	pt->header.NumPg = 2; 			/* the HF header is the second page (after the pfheader) */
	pt->header.NumFrPgFile = 0;


	/*  Code stops when using PF_AllocPage */
	error = PF_AllocPage(fd, pagenum, &headerbuf); 
	if(error != PFE_OK){
		return HFE_PF;
	}

	/* arbitratry choix of representing the info into the buffer */
	sprintf(headerbuf, "%d %d %d %d", pt->header.RecSize, pt->header.RecPage, pt->header.NumPg, pt->header.NumFrPgFile);


	/* when creating the file, the first page sould be the PFPage and the Second the HFHeader*/ 
	if( (int) pagenum != 2){
		return HFE_PF;
	}

	error = PF_UnpinPage(pt->fd, (int) pagenum, 1);
	if(error != PFE_OK){
		return HFE_PF;
	}


	error = PF_CloseFile(pt->fd);
	if(error != PFE_OK){
		return HFE_PF;
	}
	
	HFftab_length--;

	return HFE_OK;
}

int HF_DestroyFile(char *filename){
	return 0;
}

int HF_OpenFile(char *filename){
	return 0;
}

int	HF_CloseFile(int fileDesc){
	return 0;
}

RECID HF_InsertRec(int fileDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_DeleteRec(int fileDesc, RECID recId){
	return 0;
}

RECID HF_GetFirstRec(int fileDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

RECID HF_GetNextRec(int fileDesc, RECID recId, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_GetThisRec(int fileDesc, RECID recId, char *record){
	return 0;
}

int HF_OpenFileScan(int fileDesc, char attrType, int attrLength, int attrOffset, int op, char *value){
	return 0;
}

RECID HF_FindNextRec(int scanDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_CloseFileScan(int scanDesc){
	return 0;
}

void HF_PrintError(char *errString){
	printf("\nHF_PrintError : %s\n", errString);
}

bool_t HF_ValidRecId(int fileDesc, RECID recid){
	return TRUE;
}
