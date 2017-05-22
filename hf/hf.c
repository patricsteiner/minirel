#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "minirel.h"
#include "../pf/pfUtils.h"
#include "pf.h"
#include "hf.h"
#include "../hf/hfUtils.h"

HFftab_ele *HFftab;
HFscantab_ele *HFscantab;
size_t HFftab_length;
size_t HFscantab_length;
void printByte(char n){
	int j;
	for(j = 0; j < 8; j++){
		printf("%d", (n & (int) pow(2,j)) >> j);
	}
	printf(" ");
}


void printPageDirectory(HFftab_ele* pt){
	int i,size;
	size = PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char);

	printf("\nPage directory of %s\n", pt->fname);
	for (i = 0; i < size; i++){
		if(pt->header.pageDirectory[i] == 0){
			break;
		}
		printf("%d|", pt->header.pageDirectory[i]);
	}
	printf("\n");

}

void HF_PrintDataPage(char* buf, HFftab_ele* pt){
	int i,j, N, bitmapsize;
	char record[pt->header.rec_size];

	bitmapsize = (pt->header.rec_page%8)==0 ?(pt->header.rec_page / 8): (pt->header.rec_page/8)+1;

	printf("\nBITMAP\t");

	memcpy((int *) &N, (char*) ( buf + bitmapsize), sizeof(int));
	printf("Slots: %d / %d \n", N, pt->header.rec_page);
	
	for(i=0; i<bitmapsize; i++){
		printByte(buf[i]);
	}

	printf("\nRECORDS\n");
	for(i = 0; i < pt->header.rec_page; i++){
		memcpy((int *) &N, (char*) ( buf + bitmapsize ), sizeof(int));
		memcpy(record, (char*) (buf + bitmapsize + sizeof(int) + i * pt->header.rec_size), pt->header.rec_size);
		if(record[0] != 0){
			printf("slot %d\t: %s\n",i, record);
		}
	}
	printf("\n\n");	
}



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
	HFscantab= malloc(sizeof(HFscantab_ele)*MAXSCANS);
	HFftab_length = 0;
	HFscantab_length=0;
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
	if(fd < 0){
		PF_ErrorHandler(error);
	}


	/* fill the array of the hf file table*/ 
	if(HFftab_length >= HF_FTAB_SIZE){
		return HFE_FTABFULL;
	}
	pt = HFftab + HFftab_length;
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
	

	/* the page is dirty now ==> last arg of PF_UnpinPage ("dirty") set to one */

	pt->valid = FALSE;

	error = PF_UnpinPage(pt->fd, *pagenum, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}

	/*
	printf("close file\n \n");
	*/
	error = PF_CloseFile(pt->fd);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	HFftab_length--;

	return HFE_OK;
}


/* This function destroys the file whose name is fileName by calling PF_DestroyFile(). 
 * It is included here only for completeness of the HF layer interface. This function 
 * returns HFE_OK if the file is successfully destroyed, and HFE_PF otherwise. 
 * Dev: Patric
 */
int HF_DestroyFile(char *filename) {
	int error;
	if ((error = PF_DestroyFile(filename) != PFE_OK)) {
		PF_ErrorHandler(error);
	}
	return HFE_OK;
}

/*
 * This function opens a file called fileName by calling PF_OpenFile(). 
 * Its return value is the HF file descriptor of the file if the new file has been successfully opened; 
 * an HF error code is returned otherwise. 
 * In addition to opening the file, this function will make an entry in the HF file table.
 * dev : antoine
 */
int HF_OpenFile(char *filename){
	int error, fd, fileDesc;
	HFftab_ele *pt;
	char *pagebuf;


	fileDesc = HFftab_length;
	if(fileDesc >= HF_FTAB_SIZE){
		return HFE_FTABFULL;
	}
	
	fd = PF_OpenFile(filename);
	if(fd < 0){
		PF_ErrorHandler(fd);
	}

	/* read the recc_size, num pages, num free pages which are stored on the second page of the file (index = 1) */ 
	error = PF_GetThisPage(fd, 1, &pagebuf);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}


	/* Fill the array of the hf filetable */
	/*pt = ( HFftab + sizeof(HFftab_ele)*HFftab_length) ;*/
	pt = HFftab + HFftab_length;
	
	pt->fd = fd;
	pt->valid = TRUE;

	memcpy(pt->fname, filename, sizeof(filename));
	memcpy((int*) &pt->header.rec_size, (char*) (pagebuf), sizeof(int));
	memcpy((int*) &pt->header.rec_page, (char*) (pagebuf + 4), sizeof(int));
	memcpy((int*) &pt->header.num_pages, (char*) (pagebuf + 8), sizeof(int));
	memcpy((int*) &pt->header.num_free_pages, (char*) (pagebuf + 12), sizeof(int));
	memcpy((char*) &pt->header.pageDirectory, (char*) (pagebuf + 16), PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char)); 
	
	/* 
	printf("Entry added to HF Table : %d, %d, %d, %d\n", pt->header.rec_size, pt->header.rec_page, pt->header.num_pages, pt->header.num_free_pages);
	*/
	/* need also to fill the header directory (to know where are the free pages) */

	HFftab_length ++;

	error = PF_UnpinPage(pt->fd, 1, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	
	return fileDesc;
}

/*
 *This function closes the file with the file descriptor HFfd by calling PF_CloseFile() with its PF file descriptor. If successful, 
 *deletes   the entry of this file from the table of open HF files. If there is an active scan, however, this function fails and no further
 *action is done. 
 *If the hf header has been modified, then write it on page number 1.
 *It returns HFE_OK if the file has been successfully closed, HFE_SCANOPEN if there is an active scan, an HF error code otherwise.
 *
 *dev: Paul
 */


int	HF_CloseFile(int hffd){
	HFftab_ele* pt;
	int error;
	char* pagebuf;
	int i;
	i=0;
	
	

	if (hffd < 0 || hffd >= HFftab_length) return HFE_FD;
	pt=HFftab + hffd ;
	/*check is file is not already close */
	if (pt->valid!=TRUE) return HFE_FILENOTOPEN;


	/* check the header */
	if (pt->dirty==TRUE){ /* header has to be written again */
		error=PF_GetThisPage( pt->fd, 1, &pagebuf);
		if(error!=PFE_OK)PF_ErrorHandler(error);


		/* write the header  */
		memcpy((char*) pagebuf,(int*) &pt->header.rec_size, sizeof(int));
		memcpy((char*) (pagebuf+4),(int*) &pt->header.rec_page, sizeof(int));
		memcpy((char*) (pagebuf+8),(int*) &pt->header.num_pages, sizeof(int)); 
		memcpy((char*) (pagebuf+12),(int*) &pt->header.num_free_pages , sizeof(int));
		memcpy((char*) (pagebuf + 16),(char*) &pt->header.pageDirectory, PF_PAGE_SIZE - 4*sizeof(int) - sizeof(char));


		/* the page is dirty now ==> last arg of PF_UnpinPage ("dirty") set to one */
		error = PF_UnpinPage(pt->fd, 1, 1);
		if(error != PFE_OK){
			PF_ErrorHandler(error);
		}
		
	}
	/* close the file using pf layer */
	error=PF_CloseFile(pt->fd);
	if(error!=PFE_OK)PF_ErrorHandler(error);

	/* check that there is no scan in progress involving this file*/
	/* by scanning the scan table */
	for (i=0;i<HFscantab_length;i++){
			if((HFscantab+i)->HFfd==hffd) return HFE_SCANOPEN;
	}
	

	
	/*deletion */
	/* a file can be deleted only if it is at then end of the table in order to have static file descriptor */
	/* In any case the boolean valid is set to false to precise that this file is closed */
        pt->valid==FALSE;
	if(hffd==(HFftab_length-1)){ /* it is the last file of the table */ 
		HFftab_length--;

		if(HFftab_length >0){
			HFftab_length--;
			while (HFftab_length>0 &&  ((HFftab+ HFftab_length-1)->valid==FALSE)){
				HFftab_length--; /* delete all the closed file, which are the end of the table */
			}
		}
	}
	
	return HFE_OK;
}


/*
 * This function inserts the record pointed to by record into the open file associated with HFfd. 
 * This function returns the record id (of type RECID) that is assigned to the newly inserted record 
 * if the insertion is successful.
 * An HF error code otherwise.
 * dev : antoine
 */
RECID HF_InsertRec(int fileDesc, char *record){
	RECID res;
	int bit;
	HFftab_ele *pt;
	int pagenum, recnum;
	int error;
	int bitmap_size;
	int i,j,N,broke, pos;
	char* datapagebuf;

	pagenum = 0;
	recnum = 0;

	if(fileDesc < 0 || fileDesc > HF_FTAB_SIZE){ 
		res.recnum = HFE_FD;
		res.pagenum = HFE_FD;
		return res;
	}

	pt = HFftab + fileDesc;
	/*check if file is open*/
	if (pt->valid!=TRUE) {
		res.recnum=HFE_FILENOTOPEN;
		res.pagenum=HFE_FILENOTOPEN;
		return res;
	}
	if (pt->valid == FALSE){ 
		res.recnum = HFE_FILENOTOPEN;
		res.pagenum = HFE_FILENOTOPEN;
		return res;
	}

	bitmap_size = (pt->header.rec_page%8)==0 ?(pt->header.rec_page / 8): (pt->header.rec_page/8)+1;
	
	/* FIND A PAGE */
	if(pt->header.num_free_pages == 0){
		/* Allocate a new page
		 *	Get a new page
		 *	Write a null bitmap
		 *	Write 1 as number of Slots 
		 *	Update the bitmap
		 *	Write the record in first position
		 *	Increase the number of free pages
		 *	Increase the number of page
		 *	Update the pageDirectory ### USELESS , update it only when page full
		 */

		/* write the datapagebuf 
		 - bitmap: | 00000001 0000000 .... 00000000 (bytes)| 1 (int)| record (char) | | |
		 */

		error = PF_AllocPage(pt->fd, &pagenum, &datapagebuf);
		if(error != PFE_OK){PF_ErrorHandler(error);}
		
		for (i=0;i<bitmap_size;i++) {
			datapagebuf[i]=0;
		}

		datapagebuf[0] |= (int) pow(2,0);
		recnum = 0;
		N = 1;
		memcpy(( char*) ( datapagebuf + bitmap_size ), (int*) &N, sizeof(int)); /* number of slots full in the page */
		memcpy(( char*) ( datapagebuf + bitmap_size + sizeof(int) ), record, (pt->header.rec_size)); 

		pt->header.num_free_pages ++;
		(pt->header.num_pages) ++; 
		pt->header.pageDirectory[pt->header.num_pages - 3] = 0;
		pt->dirty = TRUE;
		/* printf("\nNEW DATA PAGE ALLOCATED, now %d datapages\n", pt->header.num_pages -2);
		printPageDirectory(pt);
		*/
		

	}else{ 

		broke = 0;
		/* Find a page with freespace== -1 */
		for(i=0; i < pt->header.num_pages-2 | broke ; i++){
			if(pt->header.pageDirectory[i] == 0){
				pagenum = i + 2 ;
				/*printf("\nSpace available on page %d\n", pagenum );*/
				broke = 1;
				break;
			}
		}
		if( !broke ){
			printf("PAGE DIRECTORY IS FULL");
			exit(-1);
		}
		/* find a free slot on this page */
		error = PF_GetThisPage(pt->fd, pagenum, &datapagebuf);
		if(error != PFE_OK){PF_ErrorHandler(error);}

		/* Bitmap shoudln't be full in theory
		 * for each bytes in bitmap, check each bit, 
		 * if bit = 0:
		 *	update the bitmap 
		 *	update the number of used slots (if == max number of slots, update the page directory)
		 *	insert the record at the correct position
		 */
		broke = 0;
		for(i = 0; i < bitmap_size & !broke; i++){
			for(j = 0; j < 8; j++){
				bit = (datapagebuf[i] & (int) pow(2,j) ) >> j;
				if(bit == 0){
					datapagebuf[i] |= (int) pow(2,j);
					broke = 1;
					recnum = 8*i + j;
					break;
				}
			}
		}

		/*printf("Insert '%s' on page %d, slot %d\n", record, pagenum, recnum);*/
		memcpy((int*)&N, (char*) (datapagebuf + bitmap_size), sizeof(int));
		N++;
		memcpy((char*) (datapagebuf + bitmap_size), (int*) &N, sizeof(int));
		memcpy((char*) (datapagebuf + bitmap_size + sizeof(int) + recnum*(pt->header.rec_size) ) , record, (pt->header.rec_size));

		if(N == pt->header.rec_page){
			pt->header.pageDirectory[pagenum-2] = 1;
			pt->header.num_free_pages --;
			pt->dirty = TRUE;
			/*printPageDirectory(pt);
			printf("num free pages : %d\n", pt->header.num_free_pages); */
		}

	}




	/* Algorithm : 
	 * Chech file desc
	 * Get the HFftab_elem associated
	 ******** FIND A PAGE *********
	 * If num_free_pages == 0
		allocate a page, write the bitmap (only zeroes), set recnum = 0 and pagenum (given by PFallocpage)
		update the HFtab_elem info 
	 * else, find the first freepage by looping through the char array (1 : full, 0 : free, -1 : not created)
	 	for the first number found, break the loop and get pagenum.
	 	if no more freespace in the char array, reach next header page (numPageHeader + len_bitmap + 1) (repeat under freespace is found) 
	 
	 ******** FIND A FREE RECNUM ***********
	 * Once we found a page, get the bitmap, and the remaining space
	 * look for a free recnum (1 : full, 0 : free) using a readbytes mask function
	 * write the record into the appropriate recnum (check the size of the record)
	 * write the chage into the bitmap
	 * update the remainig slots (if full, update the correct char array and hftab_elem_info)
	 * 
	 * return RECID res with the appropriate recnum and pagenum 
	 */
	


	error = PF_UnpinPage(pt->fd, pagenum, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	/********HF_PrintDataPage(datapagebuf,pt); */
	res.recnum = recnum;
	res.pagenum = pagenum;
	return res;
}

/*
 * This function deletes the record indicated by recId from the file associated with HFfd. 
 * It returns HFE_OK if the deletion is successful.
 * An HF error code otherwise.
 * dev : antoine
 */
int	HF_DeleteRec(int fileDesc, RECID recId){
	HFftab_ele* pt;
	char* datapagebuf;
	int error, bitmap_size, N;

	if( !HF_ValidRecId(fileDesc, recId) ) return HFE_INVALIDRECORD;

	pt = HFftab + fileDesc;
	/* check if file is open*/
	if (pt->valid!=TRUE) return HFE_FILENOTOPEN;
	bitmap_size = (pt->header.rec_page%8)==0 ?(pt->header.rec_page / 8): (pt->header.rec_page/8)+1;

	error = PF_GetThisPage(pt->fd, recId.pagenum, &datapagebuf);
	if(error != PFE_OK) PF_ErrorHandler(error);

	/* Update the apppropriate bit */ 
	datapagebuf[recId.recnum/8] &= ~((int) pow(2, recId.recnum % 8));

	/* update page directory and # occupied slots */ 
	memcpy((int*)&N, (char*) (datapagebuf + bitmap_size), sizeof(int));
	if(N == pt->header.rec_page){
		pt->header.num_free_pages ++;
		pt->header.pageDirectory[recId.pagenum-2] = 0;
		pt->dirty = TRUE;
	}
	N--;

	memcpy((char*) (datapagebuf + bitmap_size), (int*) &N, sizeof(int));
	/*
	HF_PrintDataPage(datapagebuf, pt);
	*/
	
	
	error = PF_UnpinPage(pt->fd, recId.pagenum, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}

	return HFE_OK;
}

/*
 *   int     HFfd;              HF file descriptor of an open file
 *   char    *record;           pointer to the record buffer
 *
 * This function retrieves a copy of the first record in the file associated with HFfd. 
 * If it succeeds, the record id of the first record in the file is returned, and a copy 
 * of the record itself is placed in the location pointed by record. If the file is empty,
 * it returns HFE_EOF, otherwise it returns an HF error code.
 * Dev: Patric
 */
RECID HF_GetFirstRec(int fileDesc, char *record) {
	RECID r;
	int error;
	r.pagenum = 2;
	r.recnum = 0;
	if ((error = HF_GetThisRec(fileDesc, r, record)) != HFE_OK) {
		r.recnum=error;
		r.pagenum=error;
		return r;
	}
	return r;
}

/*
 *   int     HFfd;              HF file descriptor of an open file
 *   RECID   recId;             record id whose next one will be retrieved
 *   char    *record;           pointer to the record buffer
 *
 * This function retrieves a copy of the record following the one with id recId in the file associated with HFfd. If it succeeds, the record id of this record is returned, and a copy of the record itself is placed in the location pointed to by record. If there are no more records in the file, it returns HFE_EOF. If the incoming record id is invalid, it returns HFE_INVALIDRECORD, otherwise it returns another HF error code.
 * Dev: Patric
 */
RECID HF_GetNextRec(int fileDesc, RECID recId, char *record) {
	/* Procedure: 
		- check if valid record
		- increment recnum
		- if recId.recnum the last record: set recnum to 0 and increment pagenum
		-call HF_GetThisRec
	*/
	int error;
	HFftab_ele* hfftab_ele;
	char* pagebuf;
	char bit;
	size_t bytes_in_bitmap;

	

	/* parameter checking */
	if(fileDesc >= HF_FTAB_SIZE || fileDesc<0) {
			recId.recnum= HFE_FD;
			recId.pagenum=HFE_FD;
			return recId;
	}
	/*initialization */
	
	hfftab_ele = HFftab + fileDesc;

	bytes_in_bitmap = HF_GetBytesInBitmap(hfftab_ele->header.rec_page);
	if( recId.recnum>(hfftab_ele->header.rec_page-1) || recId.recnum<0 ||recId.pagenum>= hfftab_ele->header.num_pages || recId.pagenum<2){
	                recId.recnum= HFE_INVALIDRECORD;
			recId.pagenum=HFE_INVALIDRECORD;
			return recId;
	}
	
	/*go to the next record */
	recId.recnum++;
	if (recId.recnum >= hfftab_ele->header.rec_page) {

		if(recId.pagenum >= (hfftab_ele->header.num_pages) ){ /*no more record in the file*/
				recId.recnum = HFE_EOF;
				recId.pagenum = HFE_EOF;
				return recId;

		}
		recId.recnum = 0;
		recId.pagenum++;
	}
	error = PF_GetThisPage(hfftab_ele->fd, recId.pagenum, &pagebuf);
	if (error != PFE_OK) PF_ErrorHandler(error);

	while(1){
	
		
		if (recId.recnum >= hfftab_ele->header.rec_page) {/* empty slot go to next one */
			/* if last slot of the file, return error*/
			if(recId.pagenum >= (hfftab_ele->header.num_pages-1)){
				/*unpin the current page and return */
				error = PF_UnpinPage(hfftab_ele->fd, recId.pagenum, 0);
				if(error != PFE_OK) PF_ErrorHandler(error);
				recId.recnum = HFE_EOF;
				recId.pagenum = HFE_EOF;
				
				return recId;

			}
			/* otherwise go to first slot of next page, by unpinning the current and get the next */
			recId.recnum = 0;
			recId.pagenum++;
			error = PF_GetThisPage(hfftab_ele->fd, recId.pagenum, &pagebuf);
			if (error != PFE_OK) PF_ErrorHandler(error);
		}
		bit = (pagebuf[recId.recnum/8] & (int) pow(2,(recId.recnum%8)) ) >> (recId.recnum%8);
		
		if ( bit!=1){
			/* no record at this number ==> go to next one*/
			recId.recnum++;
			continue;
		} 
		memcpy((char*) record, (char*) (pagebuf + bytes_in_bitmap + sizeof(int) + hfftab_ele->header.rec_size * recId.recnum), hfftab_ele->header.rec_size);

		error = PF_UnpinPage(hfftab_ele->fd, recId.pagenum, 0);
		if(error != PFE_OK) PF_ErrorHandler(error);

		return recId;
	}
}

/*
 *   int     HFfd;              HF file descriptor of an open file
 *   RECID   recId;             id of the record that will be retrieved
 *   char    *record;           pointer to the record buffer (copy)
 *
 * This function retrieves a copy of the record with recId from the file associated 
 * with HFfd. The data is placed in the buffer pointed to by record. It returns HFE_OK 
 * if it succeeds, HFE_INVALIDRECORD if the argument recId is invalid, HFE_EOF if the 
 * record with recId does not exist, or another HF error code otherwise.
 * Dev: Patric
 */
int	HF_GetThisRec(int fileDesc, RECID recId, char *record){
	/* Procedure:
		- Get the PFpage
		- Calculate what position to read from page
		- Read from page and copy it in record
	*/

	int error;
	HFftab_ele* hfftab_ele;
	size_t bytes_in_bitmap;
	char* pagebuf;
	

	
	
	/* parameter checking */
	if (record == NULL) {
		return HFE_WRONGPARAMETER;
	}

	if(fileDesc >= HF_FTAB_SIZE || fileDesc<0) return HFE_FD;
	/*initialization */
	
	hfftab_ele = HFftab + fileDesc;
	bytes_in_bitmap = HF_GetBytesInBitmap(hfftab_ele->header.rec_page);

	if( recId.recnum>(hfftab_ele->header.rec_page-1) || recId.recnum<0 ||recId.pagenum>= hfftab_ele->header.num_pages || recId.pagenum<2){
	                return HFE_INVALIDRECORD;
	}
	

	if (HF_ValidRecId(fileDesc, recId) != TRUE) {
		return HFE_INVALIDRECORD;
	}



	error = PF_GetThisPage(hfftab_ele->fd, recId.pagenum, &pagebuf);
	if (error != PFE_OK) PF_ErrorHandler(error);

	

	/*printf("\nDEBUG: rec_page: %d\n", hfftab_ele->header.rec_page);
	printf("\nDEBUG: rec_size: %d\n", hfftab_ele->header.rec_size);
	printf("\nDEBUG: recnum: %d\n", recId.recnum);
	printf("\nDEBUG: pagenum: %d\n", recId.pagenum);
	printf("\nDEBUG: bib: %d\n", bytes_in_bitmap);
	printByte(pagebuf[0]);
	/*HF_PrintDataPage(pagebuf, hfftab_ele);*/

	/* add sizeof(int) to offset because a page has bitmap first, then an int to indicate number of slots */
	memcpy((char*) record, (char*) (pagebuf + bytes_in_bitmap + sizeof(int) + hfftab_ele->header.rec_size * recId.recnum), hfftab_ele->header.rec_size);

	error = PF_UnpinPage(hfftab_ele->fd, recId.pagenum, 0);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}

	return HFE_OK;
}


/*
 *
 *This function opens a scan over the file associated with HFfd in order to return its records whose value of the indicated attribute satisfies the specified condition. 
 *The attrType field represents an attribute type, which can be a character string of length 255 or less (c), an integer (i), or a float (f). 
 *If value is a null pointer, then a scan of the entire file is desired. Otherwise, value will point to the (binary) value that records are to be compared with. 
 *The scan descriptor returned is an index into the scan table (implemented and maintained by your HF layer code) used for keeping track of information about the state of in-progress file scans. Information such as the record id of the record that was just scanned, what files are open due to the scan, etc., are to be kept in this table. If the scan table is full, an HF error code is returned in place of a scan descriptor.
 *dev: Paul
 */
int HF_OpenFileScan(int hffd, char attrType, int attrLength, int attrOffset, int op, char *value){
	HFscantab_ele* pt;
	HFftab_ele* ptfile;
	
	
	if (hffd < 0 || hffd >= HFftab_length) return HFE_FD;
	ptfile= HFftab + hffd;
	
	if (ptfile->valid!=TRUE) return HFE_FILENOTOPEN;

	if(HFscantab_length >= HF_FTAB_SIZE){
		return HFE_STABFULL;
	}
	pt=HFscantab+HFscantab_length;
	
	/* parameters checking */
	if ( attrType!='c'&& attrType !='f' && attrType!='i') return HFE_ATTRTYPE;
	if ( attrLength<=0) return HFE_ATTRLENGTH;
	if ( attrOffset<0 || attrOffset>=ptfile->header.rec_size) return HFE_ATTROFFSET;
	if( op >6 || op<1) return HFE_OPERATOR;
	
	pt->attrType=attrType;
	pt->attrLength=attrLength;
	pt->attrOffset=attrOffset;
	pt->op=op;
	memcpy( (char*) pt->value, (char*)value, attrLength);
	pt->current.recnum=0;
	pt->current.pagenum=2;
	pt->HFfd=hffd;
	pt->valid=TRUE;

	HFscantab_length++;

	return HFscantab_length-1;
}

/*
 *This function retrieves a copy of the next record in the file being scanned through scanDesc that satisfies the scan predicate. If it succeeds, it returns the record id of this record and places a copy of the record in the location pointed to by record. It returns HFE_EOF if there are no records left to scan in the file, and an HF error code otherwise.
 *
 *dev:Paul
 */

RECID HF_FindNextRec(int scanDesc, char *record){
	HFscantab_ele* ptscan;
	HFftab_ele* ptfile;
	RECID res;
	float f; /* for attribute of type float */
	int bit;
	int comp;
	char *offrecord; /* for attribute of type str and int, used in strncomp */
	int bitmap_size;
	int error;
	int pagenum;
	int recnum;
	char* pagebuf;
	char *recordbuff;
	
	/* initialisation*/
	comp=-1; 

	


	/*parameter checking */
	if (scanDesc < 0 || (scanDesc >= HFscantab_length && HFscantab_length !=0) ) {
			res.recnum = HFE_EOF;
			res.pagenum = HFE_EOF;
			return res;

		}
	if( record==NULL) {
			res.recnum = -150;
			res.pagenum = -150;
			return res;

		}
	ptscan=HFscantab+scanDesc;
	ptfile=HFftab+(ptscan->HFfd);

	bitmap_size = (ptfile->header.rec_page%8)==0 ?(ptfile->header.rec_page / 8): (ptfile->header.rec_page/8)+1;

	/*check if scan is open */
	if (ptscan->valid!=TRUE) {
		res.recnum=HFE_SCANNOTOPEN;
		res.pagenum=HFE_SCANNOTOPEN;
		return res;
	}
	pagenum=ptscan->current.pagenum;
	recnum=ptscan->current.recnum;

	error=PF_GetThisPage(ptfile->fd, pagenum, &pagebuf);
	if(error!=PFE_OK) PF_ErrorHandler(error);
	
	/* for the buffer of the record fiel being compared */
	offrecord=malloc( ptscan->attrLength);
	
	
	while(1){
		if(pagenum >= (ptfile->header.num_pages-1) && recnum >= (ptfile->header.rec_page-1)){
			error = PF_UnpinPage(ptfile->fd, pagenum, 0);
			if(error != PFE_OK) PF_ErrorHandler(error);
			
			res.recnum = HFE_EOF;
			res.pagenum = HFE_EOF;
			return res;

		}
		
		if( recnum >= ((ptfile->header.rec_page)-1)){
			error = PF_UnpinPage(ptfile->fd, pagenum, 0);
			if(error != PFE_OK) PF_ErrorHandler(error);
			
			error=PF_GetNextPage(ptfile->fd,&pagenum,&pagebuf);
			if(error!=PFE_OK) PF_ErrorHandler(error);
			recnum=0;
		}  

		recnum++;
	
		/* check if record is really a full slot with bitmap*/
		
		bit = (pagebuf[recnum/8] & (int) pow(2,(recnum%8)) ) >> (recnum%8);
		
		if ( bit!=1){
			/* no record at this number ==> go to next one*/
			
			recnum++;
			continue;
		} 
		/* now it is assured that the slot "recnum" is full */ 
		/*get this record and do the comparison */
		memcpy((char*) record,(char*) (pagebuf+bitmap_size+ sizeof(int) + recnum*(ptfile->header.rec_size) ) ,  (ptfile->header.rec_size));
		
		/* comparison steps: -apply offset
				    -switch case op
				    -comparison with value for string with strcomp
				    -comparison with  for int or float with classic operatior (< > = ) */
		/* use strncmp for int and string to save computations, does not work for float */
		
		if(ptscan->attrType !='f') memcpy(offrecord, (char*) (pagebuf+bitmap_size + sizeof(int) + recnum * ptfile->header.rec_size), ptfile->header.rec_size);
		else memcpy((float*) &f, (char*) record+(ptscan->attrOffset), ptscan->attrLength);
		
		
	
		if ( ptscan->attrType !='f') comp=strncmp((char*)offrecord, (char*) ptscan->value, ptscan->attrLength+1);
  
		
		switch(ptscan->op) {
			case 1: 
				
				if ( (ptscan->attrType == 'f' && f==*((float*)ptscan->value)) || comp==0 ){
					/*matching record is found, udpate the scan table element and then return */
					ptscan->current.recnum=recnum;
					ptscan->current.pagenum=pagenum;
					res.recnum=recnum ;
					res.pagenum=pagenum;
					free(offrecord);
					error = PF_UnpinPage(ptfile->fd, pagenum, 0);
					if(error != PFE_OK) PF_ErrorHandler(error);
			
					return res; /* copy of the record is already in the parameter, now the recid is returned*/
					}

		              	break;
			case 2:
				if ( (ptscan->attrType == 'f' && f<*((float*)ptscan->value)) || comp<0 ){
					/*matching record is found, udpate the scan table element and then return */
					ptscan->current.recnum=recnum;
					ptscan->current.pagenum=pagenum;
					res.recnum=recnum ;
					res.pagenum=pagenum;
					free(offrecord);
					error = PF_UnpinPage(ptfile->fd, pagenum, 0);
					if(error != PFE_OK) PF_ErrorHandler(error);
			
					return res; /* copy of the record is already in the parameter, now the recid is returned*/
					}
				break;
			case 3:
				/* comp initialized as negative , as to separate case where the attribute is of type float otherwise the predicat is always true*/
				if(ptscan->attrType == 'f'){
				
					if (  f<=*((float*)ptscan->value)  ){
						/*matching record is found, udpate the scan table element and then return */
						ptscan->current.recnum=recnum;
						ptscan->current.pagenum=pagenum;
						res.recnum=recnum ;
						res.pagenum=pagenum;
						free(offrecord);
						error = PF_UnpinPage(ptfile->fd, pagenum, 0);
						if(error != PFE_OK) PF_ErrorHandler(error);
			
						return res; /* copy of the record is already in the parameter, now the recid is returned*/
						}
				}
				else{
					if (   comp<=0 ){
						/*matching record is found, udpate the scan table element and then return */
						ptscan->current.recnum=recnum;
						ptscan->current.pagenum=pagenum;
						res.recnum=recnum ;
						res.pagenum=pagenum;
						free(offrecord);
						error = PF_UnpinPage(ptfile->fd, pagenum, 0);
						if(error != PFE_OK) PF_ErrorHandler(error);
			
						return res; /* copy of the record is already in the parameter, now the recid is returned*/
						}
				}
					
				break;
			case 4: 
				/* comp initialized as negative , as to separate case where the attribute is of type float otherwise the predicat is always true*/
				if(ptscan->attrType == 'f'){
				
					if (  f<=*((float*)ptscan->value) ){
						/*matching record is found, udpate the scan table element and then return */
						ptscan->current.recnum=recnum;
						ptscan->current.pagenum=pagenum;
						res.recnum=recnum ;
						res.pagenum=pagenum;
						free(offrecord);
						error = PF_UnpinPage(ptfile->fd, pagenum, 0);
						if(error != PFE_OK) PF_ErrorHandler(error);
			
						return res; /* copy of the record is already in the parameter, now the recid is returned*/
				}		}
				else{
					if (   comp<=0 ){
						/*matching record is found, udpate the scan table element and then return */
						ptscan->current.recnum=recnum;
						ptscan->current.pagenum=pagenum;
						res.recnum=recnum ;
						res.pagenum=pagenum;
						free(offrecord);
						error = PF_UnpinPage(ptfile->fd, pagenum, 0);
						if(error != PFE_OK) PF_ErrorHandler(error);
			
						return res; /* copy of the record is already in the parameter, now the recid is returned*/
						}
				}	
				break;
			case 5: 
				if ( (ptscan->attrType == 'f' && f>=*((float*)ptscan->value)) || comp>=0 ){
					/*matching record is found, udpate the scan table element and then return */
					ptscan->current.recnum=recnum;
					ptscan->current.pagenum=pagenum;
					res.recnum=recnum ;
					res.pagenum=pagenum;
					free(offrecord);
					error = PF_UnpinPage(ptfile->fd, pagenum, 0);
					if(error != PFE_OK) PF_ErrorHandler(error);
			
					return res; /* copy of the record is already in the parameter, now the recid is returned*/
					}
				break;
			case 6:
				if ( (ptscan->attrType == 'f' && f!=*((float*)ptscan->value) ) || comp!=0 ){
					/*matching record is found, udpate the scan table element and then return */
					ptscan->current.recnum=recnum;
					ptscan->current.pagenum=pagenum;
					res.recnum=recnum ;
					res.pagenum=pagenum;
					free(offrecord);
					error = PF_UnpinPage(ptfile->fd, pagenum, 0);
					if(error != PFE_OK) PF_ErrorHandler(error);
			
					return res; /* copy of the record is already in the parameter, now the recid is returned*/
					}
				break;
			default: 
				free(offrecord);
				res.recnum = HFE_OPERATOR;
				res.pagenum = HFE_OPERATOR;
				error = PF_UnpinPage(ptfile->fd, pagenum, 0);
				if(error != PFE_OK) PF_ErrorHandler(error);
			
				return res;
				break;
			}

	
		


	}
		
	
	
}

/*
 *This function terminates the file scan indicated by scanDesc. It returns HFE_OK if it succeeds, and an HF error code otherwise.
 *
 *dev:Paul
 */
int	HF_CloseFileScan(int scanDesc){
		HFscantab_ele* ptscan;
		
		
		if (scanDesc < 0 ||  (scanDesc >= HFscantab_length && HFscantab_length !=0)) return HFE_SD;
		/* same policy than for file table,if it is last scan in table:- deleted from table by decreasing the length of 1 (and deleted all following scan with boolean at FALSE 
										- otherwise put boolean valid to false*/
		ptscan=HFscantab+scanDesc;
		if (ptscan->valid==FALSE) return HFE_SCANNOTOPEN; 
		if ( scanDesc==HFscantab_length-1){
			     if (HFscantab_length>0) HFscantab_length--;
			     while( ((HFscantab+HFscantab_length-1)->valid==FALSE) && (HFscantab_length>0)) {
				 	HFscantab_length--;
				}
		}
		
																
	return 0;
}

void HF_PrintError(char *errString){
	printf("\nHF_PrintError : %s\n", errString);
}

/*
 * check if the recsid is valid, 
 * numpage<(number of pages in the file) and recnum<(max number of records in a page) 
 * Invalid if :
 		wrong filedesc
 		pagenum is not in the range of the datapages
 		recnum not it the range of the recnumber
 		associated bit in bitmap is equal to 0
 
 * dev : antoine
 */
bool_t HF_ValidRecId(int fileDesc, RECID recid){
	HFftab_ele* pt;
	char* datapagebuf;
	int bitmap_size, error, bit;
	char byte;

	if(fileDesc >= HF_FTAB_SIZE || fileDesc<0) return FALSE;
	pt = HFftab + fileDesc;

	if(pt->valid == FALSE) return FALSE;

	if(recid.pagenum < 2 || recid.pagenum > pt->header.num_pages){
		/*printf(" pagenum , num pages dans valid %d, %d\n", recid.pagenum, pt->header.num_pages);
		printf("Invalid pagenum number\n");*/
		return FALSE;
	}
	if(recid.recnum < 0 || recid.recnum > pt->header.rec_page){
		return FALSE;
	}
	bitmap_size = (pt->header.rec_page%8)==0 ?(pt->header.rec_page / 8): (pt->header.rec_page/8)+1;

	error = PF_GetThisPage(pt->fd, recid.pagenum, &datapagebuf);

	if(error != PFE_OK) return FALSE;

	byte = datapagebuf[recid.recnum/8];
	bit = (byte & (int) pow(2, recid.recnum%8)) >> recid.recnum%8;
 	
	error = PF_UnpinPage(pt->fd, recid.pagenum, 0);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}
	

	if(bit) return TRUE;

	return FALSE;
}

/*
 * Calculates the amount of bytes in the bitmap, given the amount of records per page. 
 */
int HF_GetBytesInBitmap(int records_per_page) {
	int r;
	r = records_per_page / 8;
	if (records_per_page % 8 != 0) r++;
	return (size_t)r;
}

void HF_PrintTable(void){
	size_t i;
	printf("\n\n******** HF Table ********\n");
	printf("******* Length : %d *******\n",(int) HFftab_length);
	for(i=0; i<HFftab_length; i++){
		printf("**** %d : %s : %d freepages : %d valid\n", (int)i, HFftab[i].fname, HFftab[i].header.num_free_pages, HFftab[i].valid);
	}
	printf("**************************\n\n");

	PF_PrintTable();
}

void HF_ErrorHandler(int error){
	switch(error) {
		case HFE_INVALIDRECORD: printf("\n HF: Invalid record/fileDesc combination \n"); break;
		default: printf( " \n  HF: unused error code : %d \n\n ", error);
	}
	exit(-1);
}
		
