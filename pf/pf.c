#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "minirel.h"
#include "bf.h"
#include "../bf/bfUtils.h"
#include "pf.h"
#include "pfUtils.h"


PFftab_ele *PFftab; 
size_t PFftab_length;

void PF_PrintTable(void){
	size_t i;
	printf("\n\n******** PF Table ********\n");
	printf("******* Length : %d *******\n",(int) PFftab_length);
	for(i=0; i<PFftab_length; i++){
		printf("* %d : %s : %d pages : %d valid *\n", (int) i, PFftab[i].fname, PFftab[i].hdr.numpages, PFftab[i].valid);
	}
	printf("**************************\n\n");

	BF_ShowBuf();
}
/*
 * Init the PF layer and also the BF layer by BF_Init
 * Creates and initialized the PFftab: file table of PFfile
 * Dev : Paul
 */
void PF_Init(void){
	PFftab=malloc(sizeof(PFftab_ele)*PF_FTAB_SIZE); /*PFftab is an array of PF_FTAB_SIZE PFf_tabe_ele elements. */ 
	PFftab_length=0; /*the file table is still empty;*/
	BF_Init();
	
	
}

/*
 * Creates File and then create pffile table entry, writes the header of the file and close it.
 * Dev : Paul
 */



int PF_CreateFile(char *filename){
	int unixfd;
	int PFfd;
	ino_t inode;
	int ret;  
	struct stat file_stat;  
	PFftab_ele* pt;
	PFhdr_str* hdr;
	
	/*parameters for calling BF_Layer*/
	PFpage* fpage;
	BFreq breq;


	unixfd=0;
	PFfd=0;
	
    /* set the rights (read write) pf the file when created */
	unixfd=open(filename, O_CREAT|O_RDWR, 0666);
	
	if (unixfd < 0) {  
	    /* problem occurred while opening the file */
	    return PFE_FILENOTOPEN;
	}   
        /* inode number is necessary to create PFftab entry */

	ret = fstat(unixfd, &file_stat);  
	if (ret!=0) return PFE_UNIX; 

	inode = file_stat.st_ino; 
	
	/*fill the next array of the PF file table*/
	if (PFftab_length >= PF_FTAB_SIZE) {
		return PFE_FTABFULL;
	}
	pt=(PFftab+PFftab_length);

	pt->valid=TRUE;
	pt->inode=inode;
	sprintf(pt->fname,"%s",filename);/* à tester */
	pt->unixfd=unixfd;
	pt->hdr.numpages=1; /* first page is for the header */
	pt->hdrchanged=FALSE; 
	
	/* pf file descriptor is the index in the table */
	PFfd=PFftab_length;
	PFftab_length++;

	/* hdrchanged is false, because next step is the copy of the header in the first page of the file*/
	/* file header is written in the first page of the file */
	breq.fd=PFfd;
	breq.unixfd=unixfd;
	breq.pagenum=0;
	breq.dirty=FALSE;
	
	ret=BF_AllocBuf(breq, &fpage);
	if(ret!=0) BF_ErrorHandler(ret);/*print a chosen string and then exit */
	
	/*sprintf(fpage->pagebuf, "%d", pt->hdr.numpages); /*all the page is for header of file == number of pages in the page.*/
	memcpy((char*) fpage->pagebuf, (int *)&(pt->hdr.numpages), sizeof(int));
	breq.dirty=TRUE;
	pt->valid=FALSE;

	ret=BF_TouchBuf(breq); /* page is dirty */
	if(ret!=0) BF_ErrorHandler(ret);

	ret= BF_UnpinBuf(breq); /* page has pinned to 0 now, we can flush the file */
	if(ret!=0) BF_ErrorHandler(ret);

	ret= BF_FlushBuf(PFfd);
	if(ret!=0) BF_ErrorHandler(ret); /* the header is on disk drive */
	
	/* final step close the file and remove it from PFftable*/
	PFftab_length--; /* enough to remove it for the table */
	ret=close(unixfd);
	
	if (ret < 0) {  
	    /* problem occurred while closing the file */
	    return PFE_UNIX;
	}

	return PFE_OK;
}	

/*
 * This function destroys the file filename. 
 * The file should have existed, and should not be already open. 
 * This function returns PFE_OK if the operation is successful, an error condition otherwise.
 * Dev: Patric
 */
int PF_DestroyFile (char *filename) {
	size_t i;
	PFftab_ele* ftab_ele;
	ftab_ele = NULL;

	/* search for file in PFftab by filename*/
	for (i = 0; i < PFftab_length; i++) {
		if (strcmp((PFftab+i)->fname, filename) == 0) {
			ftab_ele = (PFftab+i); /* found the page */
			break;
		}
	}
	
	if(ftab_ele!=NULL){ /*if NULL may the file was the last of the table and has been freed */
			if (ftab_ele->valid == TRUE) { /* if file open */
				return PFE_FILEOPEN;
			}
		}
	
	if (access(filename, F_OK) == -1) { /* file does not exist */
		return PFE_FILENOTEXISTS;
	}
	else{
		unlink(filename);
		return PFE_OK; /* delete it */
	}
}

/*
 * This function opens the file named filename using the system call open(), and reads in the file header.
 * Then, the fields in the file table entry are filled accordingly and the index of the file table entry is returned.
 * This function returns a PF file descriptor if the operation is successful, an error condition otherwise.
 * Dev: Antoine
 */
int PF_OpenFile (char *filename) {
	BFreq bq;
	PFpage* fpageHeader;

	struct stat file_stat;
	int res, fd, unixfd;
	PFftab_ele* pt;

	if (PFftab_length >= PF_FTAB_SIZE) {
		return PFE_FTABFULL;
	}

	if ((unixfd = open(filename, O_RDWR))<0){
		printf("open failed: %s", filename);
		return PFE_FILENOTOPEN;
    }

   	/* prepare buffer request to get fpageHeader */
    bq.unixfd = unixfd;
    bq.fd = PFftab_length; 
    bq.pagenum = 0; /*header is the first page of a file */
    bq.dirty = FALSE;

    res = BF_GetBuf(bq, &fpageHeader); /* PIN of the file is set to 1 */
    if(res != BFE_OK) BF_ErrorHandler(res);



    /* fill the next array of the PF file table*/
    pt = ( PFftab + PFftab_length );

    /* get inode */
    if(fstat(unixfd, &file_stat)) return PFE_UNIX;
    
    /* fill the new entry */
    pt->valid = TRUE;
    pt->inode = file_stat.st_ino;
    sprintf(pt->fname,"%s",filename);
    pt->unixfd = unixfd;
    /* get the number of pages written in the pagebuf*/
    /* sprintf((char*)fpageHeader->pagebuf, "%4d", pt->hdr.numpages);*/
    memcpy((int*) &(pt->hdr.numpages), (char*)fpageHeader->pagebuf, sizeof(int) );
    pt->hdrchanged = FALSE;

    PFftab_length++; /* for next entry */

    res= BF_UnpinBuf(bq); /* page has pinned to 0 now */
	if(res!=0) BF_ErrorHandler(res);
	/*
    printf("\nThe file '%s' containing %d pages (including header page) has been added to the PFtable.\n", filename, pt->hdr.numpages);
	*/
	return bq.fd;
}

/*
 * This function closes the file associated with PF file descriptor fd
 * This entailes releasing all the buffer pages belonging to the file from the LRU list to the free list.
 * Meanwhile, dirty pages must be written back to the file if any.
 * All the buffer pages of a file must have been unpinned in order for the file to be closed successfully.
 * If the file header has changed, it is written back to the file. 
 * The file is finally closed by using the system call close(). The file table entry corresponding to the file is freed. 
 * This function returns PFE_OK if the operation is successful, an error condition otherwise.
 * Dev: Antoine
 */
int PF_CloseFile (int fd) {
	PFftab_ele* pt;
	BFreq bq;
	PFpage* fpageHeader;
	int error;

	if (fd < 0 || fd >= PFftab_length) return PFE_FD;
	pt = PFftab + fd;

	if(pt->hdrchanged=TRUE){
	    /* to write the header : write with get buf, touchbuf and unpin */ 
	    bq.unixfd = pt->unixfd;
	    bq.fd = fd ; 
	    bq.pagenum = 0; /*header is the first page of a file */
	    bq.dirty = TRUE;

	    /* get the headerpage to write into it (increases the pincount by 1) */
		error = BF_GetBuf(bq, &fpageHeader);
    	if(error != BFE_OK) BF_ErrorHandler(error);
    	/*sprintf(fpageHeader->pagebuf, "%d", pt->hdr.numpages);*/
    	memcpy((char*) fpageHeader->pagebuf, (int *)&(pt->hdr.numpages), sizeof(int));
    	/* say to buffer pool that this page is dirty*/
    	error = BF_TouchBuf(bq);
    	if(error!=BFE_OK) BF_ErrorHandler(error);

    	/* Unpin the page after */
    	error = BF_UnpinBuf(bq);
    	if (error != BFE_OK) BF_ErrorHandler(error);
	}

	/* Flush buffer before closing the file */
	error = BF_FlushBuf(fd);
	if(error != BFE_OK) BF_ErrorHandler(error);

	/* Close the file with unix close*/
	if (close(pt->unixfd) < 0) {
		printf("close failed for file '%s'", pt->fname);
		return PFE_UNIX;
         }

	/*deletion */
	/* a file can be deleted only if it is at then end of the table in order to have static file descriptor */
	/* if it is not the case then the boolean valid is set to false to precise that this file is closed */

	 pt->valid = FALSE;
	 if(fd == (PFftab_length-1)){
		
		if(PFftab_length>0){
			PFftab_length--;
			while(PFftab_length>0 && ((PFftab + (PFftab_length-1))->valid==FALSE) ) PFftab_length--;/* delete all the closed file, which are the end of the table */
        }
    }

	    /*
	    printf("\nThe file '%s' containing %d pages (including header page) has been closed.\n", pt->fname, pt->hdr.numpages);
		*/

	return PFE_OK;
}

/* 
 *    int     fd;          PF file descriptor
 *    int     *pageNum;    return the number of the page allocated
 *    char    **pagebuf;   return a pointer to the page content
 *
 * This function allocates a page in the file associated with a file descriptor fd. 
 * This new page is appended to the end of the file. 
 * This function also allocates a buffer entry corresponding to the new page. 
 * The value of pageNum for the page being allocated must be determined from the
 * information stored in the file header. The page allocated by this function is pinned and marked
 * dirty so that it will be written to the file eventually. Upon a successful page allocation, 
 * the file header must be updated accordingly. This function returns PFE_OK if the operation is
 * successful, an error condition otherwise.
 * Dev: Patric
 */
int PF_AllocPage (int fd, int *pagenum, char **pagebuf) {
	int new_pagenum;
	BFreq bq;
	PFpage* pfpage;
	int error;

	if(pagenum==NULL || pagebuf==NULL){
			return PFE_NULLARG;
	}
	if (fd < 0 || fd >= PFftab_length) return PFE_FD;
	
	new_pagenum = (PFftab + fd)->hdr.numpages;	/*or  pfftab_ele = (PFftab + sizeof(PFftab_ele) * fd);*/
	bq.fd = fd;
	bq.unixfd = (PFftab + fd)->unixfd;
	bq.dirty = TRUE;
	bq.pagenum = new_pagenum;

	if ((error = BF_AllocBuf(bq, &pfpage)) != BFE_OK) {
		BF_ErrorHandler(error);
	}
	/* make page dirty */
	if ((error = BF_TouchBuf(bq)) != BFE_OK) {
		BF_ErrorHandler(error);
	}
	/* update page info and header*/
	*pagenum = new_pagenum;
	*(pagebuf)=pfpage->pagebuf;
	(PFftab + fd)->hdrchanged = TRUE;
	(PFftab + fd)->hdr.numpages++ ;
	

	return PFE_OK;
}


/*
 *Get the page after pagenum in pagebuf and change pagenum to *pagenum+1
 *Dev: Paul
 */

int PF_GetNextPage (int fd, int *pageNum, char **pagebuf){
	int ret;  
	PFftab_ele* pt;
		
	/*parameters for calling BF_Layer*/
	PFpage* fpage;
	BFreq bq;
	
	
	/* fill bq using PFftab_ele fields */

	pt=(PFftab+fd);
	bq.fd=fd;
	bq.unixfd=pt->unixfd;
	bq.dirty=FALSE;
	bq.pagenum=(*pageNum)+1;
	
	if( pt->hdr.numpages <= (*pageNum)+1 ) return PFE_EOF; /* the wanted page does not exist */
	
	ret=BF_GetBuf(bq,&fpage);
	if(ret!=0) BF_ErrorHandler(ret);
	
	*pagebuf= (fpage->pagebuf);
	(*pageNum)++;
	
	return PFE_OK;



}

/*
 *Get the first page of the file ( the header) using PF_GetNextPage
 *Dev: Paul
 */


int PF_GetFirstPage (int fd, int *pageNum, char **pagebuf){
	
	/* if first page wanted then pageNum has to -1 */
	(*pageNum)=-1;
	
	return PF_GetNextPage(fd, pageNum, pagebuf);
}

/*
 *Get the page with this file descriptor and this page number
 *Dev: Paul 
 */


int PF_GetThisPage (int fd, int pageNum, char **pagebuf){
	int ret;  
	PFftab_ele* pt;
		
	/*parameters for calling BF_Layer*/
	PFpage* fpage;
	BFreq bq;
	
	
	/* fill bq using PFftab_ele fields */
	
	pt=(PFftab+fd);
	bq.fd=fd;
	bq.unixfd=pt->unixfd;
	bq.dirty=FALSE;
	bq.pagenum=pageNum;
	
	if( pt->hdr.numpages <= pageNum) return PFE_EOF; /* the wanted page does not exist */
	
	ret=BF_GetBuf(bq,&fpage);
	if(ret!=0) BF_ErrorHandler(ret);
	
	*pagebuf= (fpage->pagebuf);
	
	
	return PFE_OK;


}


/*
 * After checking the validity of the fd and pageNum values, 
 * this function marks the page associated with pageNum and fd dirty. 
 * It returns PFE_OK if the operation is successful, an error condition otherwise.
 * Dev: Antoine 
 */
int PF_DirtyPage(int fd, int pageNum) {
	PFftab_ele* ftab_ele;
	BFreq bq;
	int resBF;

	if (fd < 0 || fd >= PFftab_length) return PFE_FD;
	
	ftab_ele = (PFftab + fd);
        /*printf( "page num and number of page of the file %d,  %d \n", pageNum, ftab_ele->hdr.numpages);*/
	if (pageNum < 0 || pageNum >= ftab_ele->hdr.numpages){
		return PFE_INVALIDPAGE;
	}

	/* prepare buffer request */
	bq.unixfd = ftab_ele->unixfd;
    bq.fd = fd; 
    bq.pagenum = pageNum;
    bq.dirty = TRUE; /* useless */

    /* mark dirty in the buf */
    resBF = BF_TouchBuf(bq);
    if(resBF != BFE_OK) BF_ErrorHandler(resBF);

	return PFE_OK;
}

/* 
 *    int     fd            PF file descriptor
 *    int     pageNum       number of page to be unpinned
 *    int     dirty         dirty indication
 * After checking the validity of the fd and pageNum values, this function unpins the page 
 * numbered pageNum of the file with file descriptor fd. Besides, if the argument dirty is TRUE, 
 * the page is marked dirty. This function returns PFE_OK if the operation is successful, 
 * an error condition otherwise.
 * Dev: Patric
 */
int PF_UnpinPage(int fd, int pageNum, int dirty) {	
	PFftab_ele* pfftab_ele;
	BFreq bq;
	int error;

	if (fd < 0 || fd >= PFftab_length) {
		return PFE_FD;
	}
	
	pfftab_ele = (PFftab + fd); 
	
	if (pageNum < 0 || pageNum >= pfftab_ele->hdr.numpages) {
		return PFE_INVALIDPAGE;
	}
	
	bq.fd = fd;
	bq.unixfd = pfftab_ele->unixfd;
	bq.dirty = dirty;
	bq.pagenum = pageNum;
	if (dirty) { /* mark page as dirty when it should be dirty */
		if ((error = BF_TouchBuf(bq)) != BFE_OK) {
			BF_ErrorHandler(error);
		}
	}
	
	if ((error = BF_UnpinBuf(bq)) != BFE_OK) {
		BF_ErrorHandler(error);
	}

	return PFE_OK;
}


    
void PF_PrintError (char* error){

	printf( "\nPF: Error , message :%s \n",error);
	exit(-1);
}

/*
 *used in HF and AM layer to return an understandable message of error 
 */
void PF_ErrorHandler(int error){
	PF_PrintTable();
	switch( error){
		case PFE_INVALIDPAGE: printf("\n PF: the page number is negative or superior or equal than the number of page in the file(PFE_INVALIDPAGE) \n"); break;
		
		case PFE_FD: printf( " \nPF: pf file descriptor is negative or superior than the size of PF file table(PFE_FD) \n\n ");break;

		case PFE_EOF: printf( " \n PF:the wanted page does not exist, page number >= number of page in its file. (PFE_EOF)  \n\n ");break;

		case PFE_UNIX: printf(" \n PF: unix function did not succeed ( open(fd) or stat(), PFE_UNIX)\n\n ");break;

		case PFE_FILENOTOPEN:printf(" \n PF:primitive open(filename) did not succeed   \n\n");break;
	
		case PFE_FILEOPEN:printf(" \n PF: the file has to be destroy but it is still open   \n\n");break;

		case PFE_FILENOTEXISTS:printf(" \n PF:the file you ask to delete,does not exist   \n\n");break;

		case PFE_FTABFULL: printf( " \n PF: PF file table is full, impossible to open or create a file ( PFE_FTABFULL) \n\n");break;

		case PFE_NULLARG: printf( " \n PF: given argument are NULL, this error avoids a seg fault \n\n");break;

		case PFE_OK: printf( "\n PF: error handler called but no error \n\n "); break;

		default: printf( " \n  PF: unused error code : %d \n\n ", error);
	}
	exit(-1);
}
