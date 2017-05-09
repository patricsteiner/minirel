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
size_t HFftab_length;

void printByte(char n){
	int j;
	for(j = 0; j < 7; j++){
		printf("%d", (n & (int) pow(2,j)) >> j);
	}
	printf("\t");
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
	int i, N, bitmapsize;
	char record[pt->header.rec_size];

	bitmapsize = (pt->header.rec_page%8)==0 ?(pt->header.rec_page / 8): (pt->header.rec_page/8)+1;
	printf("\nBITMAP\t");

	memcpy((int *) &N, (char*) ( &buf[bitmapsize]), sizeof(int));
	printf("Slots: %d / %d \n", N, pt->header.rec_page);
	
	for(i=0; i<bitmapsize; i++){
		printByte(buf[i]);
	}
	printf("\n");

	printf("RECORDS\n");
	for(i = 0; i < pt->header.rec_page; i++){
		memcpy((int *) &N, (char*) ( &buf[bitmapsize]), sizeof(int));
		memcpy(record, (char*) (&buf[bitmapsize] + sizeof(int) + i * pt->header.rec_size), pt->header.rec_size);
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
	if(fd < 0){
		PF_ErrorHandler(error);
	}


	/* fill the array of the hf file table*/ 
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
	

	/* the page is dirty now*/

	error = PF_UnpinPage(pt->fd, *pagenum, 1);
	if(error != PFE_OK){
		PF_ErrorHandler(error);
	}

	printf("close file\n \n");
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
	printf("Entry added to HF Table : %d, %d, %d, %d\n", pt->header.rec_size, pt->header.rec_page, pt->header.num_pages, pt->header.num_free_pages);
	/* need also to fill the header directory (to know where are the free pages) */

	HFftab_length ++;
	return fileDesc;
}

int	HF_CloseFile(int fileDesc){
	return 0;
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
		memcpy(( char*) ( &datapagebuf[bitmap_size] ), (int*) &N, sizeof(int)); /* number of slots full in the page */
		memcpy(( char*) ( &datapagebuf[bitmap_size] + sizeof(int) ), record, (pt->header.rec_size)); 

		pt->header.num_free_pages ++;
		pt->header.num_pages ++;
		pt->header.pageDirectory[pt->header.num_pages - 3] = -1;
		printf("\nNEW DATA PAGE ALLOCATED, now %d datapages\n", pt->header.num_pages -2);
		printPageDirectory(pt);
		
		

	}else{ 

		broke = 0;
		/* Find a page with freespace== -1 */
		for(i=0; i < pt->header.num_pages-2 | broke ; i++){
			if(pt->header.pageDirectory[i] == -1){
				pagenum = i + 2 ;
				printf("\nSpace available on page %d\n", pagenum );
				broke = 1;
				break;
			}
		}
		if( !broke ){
			printf("NO BROKE");
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
				bit = ( datapagebuf[i] & (int) pow(2,j) ) >> j;
				if(bit == 0){
					datapagebuf[i] |= (int) pow(2,j);
					broke = 1;
					recnum = 8*i + j;
					break;
				}
			}
		}

		printf("Insert '%s' on page %d, slot %d\n", record, pagenum, recnum);
		memcpy((int*)&N, (char*) (&datapagebuf[bitmap_size]), sizeof(int));
		N++;
		memcpy((char*) (&datapagebuf[bitmap_size]), (int*) &N, sizeof(int));
		memcpy((char*) (&datapagebuf[bitmap_size] + sizeof(int) + recnum*(pt->header.rec_size) ) , record, (pt->header.rec_size));

		if(N == pt->header.rec_page){
			pt->header.pageDirectory[pagenum-2] = 1;
			pt->header.num_free_pages --;
			/*printPageDirectory(pt);*/
			printf("num free pages : %d\n", pt->header.num_free_pages);
		}

	}

	HF_PrintDataPage(datapagebuf, pt);
/*
for(i=0; i<8; i++){
bit = (int) pow(2,i);
printf("bit %d : %d\n", i, (datapagebuf[0] & bit) >> i);
}
*/
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

void HF_PrintTable(void){
	size_t i;
	printf("\n\n******** HF Table ********\n");
	printf("******* Length : %d *******\n",(int) HFftab_length);
	for(i=0; i<HFftab_length; i++){
		printf("* %d : %s : pageDirectory %s  *\n", (int)i, HFftab[i].fname, HFftab[i].header.pageDirectory);
	}
	printf("**************************\n\n");
}

