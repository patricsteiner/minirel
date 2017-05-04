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



/******************************************** way of implementation *******************************************/
/*
 *************** To know the number of rec per page, easier way it to do that loop:****************************
        int rec_page;
	x=0;
	rest= ((PF_PAGE_SIZE  - sizeof(int)) % (rec_size))*8 ;  /* it is the number of unused bit in a packed formated page
	n_packed= (PF_PAGE_SIZE  - sizeof(int)) / (rec_size); /*since it is int operands, it will be the floor of quotient == the number of rec in packed formated page 
	
	while ( (rest + x*(8*rec_size)<n_packed) x++; /* n_packed is the maximum size of the bitmap (== number of records) , if x is not 0 then the size of bitmap will be (n_packed-x)
	rec_page=n_packed-x;
	/* problem if bitmap is not multiple of 8 ( like size of bitmap will be 3,5 bytes if there is 3*8+0.5*8=28 records per page), no problem if unused bits are more than (nb_bits_in_bitmaps%8) because we can add some theses bits in the bitmap to complete the last bytes, otherwise the page can contains n_packed-x-1
	 if( (PF_PAGE_SIZE - ((rec_page*rec_size)+sizeof(int) + rec_page))< rec_page%8) rec_page--; and bitmap_size = (rec_page%8)==0 ?(rec_page / 8): (rec_page/8)+1;



 ********************* To modify bitmap with the page "pfpagebuf" **********************************************
 
  Bitmap size is equal to the number of records per page divided by 8 since unit of size is byte
  Bitmap is the first bits of the page : 
 
  example to delete the 8th record is deleted:
            pfpagebuf[0] &= 0b01111111 ; /* since way other bits of this byte are preserved and 8th bits is set to 0
	to add the 8th record  :
	     pfpagebuf[0] |=0b10000000 ;  /* +4 because the first 

	0b01111111 is equal to (2⁷-1)
	0b10000000 is equal to 2⁷ 

       so to delete the xth record:
		pfpagebuf[(x-1/8)] &= 2^((x-1)%8) - 1
       to add it :
		pfpagebuf[(x-1/8)] |= 2^((x-1)%8) 
			
   
 ****************  To modify the Number of records on the page "pfpagebuf" *******************************
  to put N as the number of full records in the page : 
	 memcpy((char*) pfpagebuf + (rec_size/8), (int*) &N , sizeof(int));
  to recuperate and increment it by one :
	memcpy((int *)&N, pfpagebuf+(rec_size/8), sizeof(int)); 
        memcpy((char*) pfpagebuf + (rec_size/8), (int*) &(N+1) , sizeof(int));


 *************** To modify a specific records to pagenbuf *************************************************
	if the record number is x:
		    memcpy((char*) (pfpagebuf + (rec_size/8)+sizeof(int)+ x*rec_size) , (char*) record , sizeof(rec_size));

 	
 ****************************************************************************************************************/


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
 *Given the size of a record, this function calculates the maximum number of records for one page 
 *errors are not possible since rec_size has been already checked by HF function calling HF_ComputeRecPage
 */

void HF_ComputeRecPage(int rec_size, int* rec_per_page){
	int rec_page=0; 
       
        int rest,n_packed,x;

	x=0;
	rest= ( (PF_PAGE_SIZE  - sizeof(int) ) % (rec_size) )*8 ;  /* it is the number of unused bits in a packed formated page*/
	n_packed= (PF_PAGE_SIZE  - sizeof(int)) / (rec_size); /*since it is int operands, it will be the floor of quotient == the number of rec in packed formated page */
	
	while ( (rest + x*(8*rec_size))<n_packed) {x++;} /* n_packed is the maximum size of the bitmap (== number of records) , if x is not 0 then the size of bitmap will be (n_packed-x)*/
	rec_page= n_packed-x;
	
	/**** problem if bitmap is not multiple of 8 ( like size of bitmap will be 3,5 bytes if there is 3*8+0.5*8=28 records per page), no problem if unused bits are more than (nb_bits_in_bitmaps%8) because we can add some theses bits in the bitmap to complete the last bytes, otherwise the page can contains n_packed-x-1 ****/

	 if( ((PF_PAGE_SIZE*8) - (8*((rec_page*rec_size)+sizeof(int)) + rec_page))< rec_page%8) rec_page--; 
	/* bitmap_size = (rec_page%8)==0 ?(rec_page / 8): (rec_page/8)+1; */
	*rec_per_page=rec_page;

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
	pagenum=malloc(sizeof(int));
	
	if( RecSize<=0 || (RecSize >= (PF_PAGE_SIZE-sizeof(int))) ){
		return HFE_RECSIZE;
	}

	error = PF_CreateFile(filename);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	/*printf("Create file\n");*/

	fd = PF_OpenFile(filename);
	if(fd != PFE_OK){
		PF_ErrorHandler(error);
	}
	printf("Open file \n");


	/* fill the array of the hf file table*/ 
	pt = HFftab + sizeof(HFftab_ele)*HFftab_length;
	HFftab_length ++;

	pt->fd = fd;
	pt->valid = TRUE;
	memcpy(pt->fname, filename, sizeof(filename)); 
	pt->header.rec_size = RecSize;
	HF_ComputeRecPage(RecSize,&pt->header.rec_page); /* number of recs per page */
	pt->header.num_pages = 2; 			/* the HF header is the second page (after the pfheader) */
	pt->header.num_free_pages = 0;


	/*  Code stops when using PF_AllocPage*/
	error = PF_AllocPage(fd, pagenum, &headerbuf); 
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	/* when creating the file, the first page number=0 sould be the PFPage and the Second number=1 the HFHeader*/ 
	if( *pagenum != 1){
		return HFE_PF;
	}

	/* arbitratry choice of representing the info into the buffer*/ /* §§§§§We should use memcpy, is nt it */
	/*sprintf(headerbuf, "%d %d %d %d", pt->header.rec_size, pt->header.rec_page, pt->header.num_pages, pt->header.num_free_pages); */
	memcpy((char*) headerbuf,(int*) &pt->header.rec_size, sizeof(int));
        memcpy((char*) (headerbuf+4),(int*) &pt->header.rec_page, sizeof(int));
	memcpy((char*) (headerbuf+8),(int*) &pt->header.num_pages, sizeof(int));	
	memcpy((char*) (headerbuf+12),(int*) &pt->header.num_free_pages , sizeof(int));
	

	/* the page is dirty now*/

	error = PF_UnpinPage(pt->fd, *pagenum, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}


	error = PF_CloseFile(pt->fd);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
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

/*
 *check if the id is valid, numpage<(number of pages in the file) and recnum<(max number of records in a page) and bit for this recnum -1 is to 1 
 *
 */
bool_t HF_ValidRecId(int fileDesc, RECID recid){
	return TRUE;
}
