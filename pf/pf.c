#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "minirel.h"
#include "bf.h"
#include "bfUtils.h"
#include "pf.h"
#include "pfHeader.h"


PFftab_ele *PFftab; 
size_t PFftab_length;



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
	
	
	unixfd=open(filename, O_CREAT|O_RDWR);
	
	if (unixfd < 0) {  
	    /* problem occurred while opening the file */
	    return PFE_FILENOTOPEN;
	}   
        /* inode number is necessary to create PFftab entry */

	ret = fstat(unixfd, &file_stat);  
	if (ret!=0) return PFE_UNIX; 

	inode = file_stat.st_ino; 
	
	/*fill the next array of the PF file table*/
	pt=(PFftab+sizeof(PFftab_ele)*PFftab_length);

	pt->valid=TRUE;
	pt->inode=inode;
	snprintf(pt->fname,  sizeof(filename),"%s", filename);/* à tester */
	pt->unixfd=unixfd;
	pt->hdr.numpages=1; /* first page is for the header */
	pt->hdrchanged=FALSE; 
	
	/* pf file descriptor is the index in the table */
	PFfd=PFftab_length;
	PFftab_length++;
	hdr->numpages=1;


	/* hdrchanged is false, because next step is the copy of the header in the first page of the file*/
	/* file header is written in the first page of the file */
	breq.fd=PFfd;
	breq.unixfd=unixfd;
	breq.pagenum=0;
	breq.dirty=FALSE;
	
	ret=BF_AllocBuf(breq, &fpage);
	if(ret!=0) BF_ErrorHandler(ret);/*print a chosen string and then exit */
	
	snprintf(fpage->pagebuf, PAGE_SIZE ,"%d", hdr->numpages); /*all the page is for header of file == number of pages in the page.*/
	breq.dirty=TRUE;

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
 * This function destroys the file filename. The file should have existed, and should not be already 
 * open. This function returns PFE_OK if the operation is successful, an error condition otherwise.
 * Dev: Patric
 */
int PF_DestroyFile (char *filename) {
	if (access(filename, F_OK) != -1) { /* if file exists */
		return (unlink(filename));
	}
	return -1; /* TODO better error code? */
}

/*
 * This function opens the file named filename using the system call open(), and reads in the file header.
 * Then, the fields in the file table entry are filled accordingly and the index of the file table entry is returned.
 * This function returns a PF file descriptor if the operation is successful, an error condition otherwise.
 * Dev: Antoine
 */

int PF_OpenFile (char *filename){
	BFreq bq;
	PFpage* fpageHeader;
	struct stat file_stat;
	int res, fd, unixfd;
	PFftab_ele* pt;

	if(PFftab_length>=PF_FTAB_SIZE){
		return PFE_FTABFULL;
	}

	if ((unixfd = open(filename, O_RDWR))<0){
		printf("open failed: %s", filename);
		return PFE_FILENOTOPEN;
    }

   	/* prepare buffer request */
    bq.unixfd = unixfd;
    bq.fd = PFftab_length ; 
    bq.pagenum = 0; /*header is the first page of a file */
    bq.dirty = FALSE;

    res = BF_GetBuf(bq, &fpageHeader);
    if(res != BFE_OK){return PFE_GETBUF;}

    /* prepare new entry */
    pt=(PFftab+sizeof(PFftab_ele)*PFftab_length);

    /* get inode */
    if(fstat(unixfd, &file_stat)) {
    	close(unixfd);
        return PFE_UNIX;
    } 
    
    /* fill the new entry */
    pt->valid = TRUE;
    pt->inode = file_stat.st_ino;
    pt->fname = filename;
    pt->unixfd = unixfd;
    pt->hdr.numpages = atoi(fpageHeader->pagebuf);
    pt->hdrchanged = FALSE;

    PFftab_length++; /* for next entry */

    printf("\nThe file '%s' containing %d pages has been added to the PFtable.\n", filename, pt->hdr.numpages);
	return fd;
}

/*
 * This function closes the file associated with PF file descriptor fd
 * This entailes releasing all the buffer pages belonging to the file from the LRU list to the free list.
 * Meanwhile, dirty pages must be written back to the file if any.
 * All the buffer pages of a file must have been unpinned in order for the file to be closed successfully.
 * If the file header has changed, it is written back to the file. 
 * The file is finally closed by using the system call close(). The file table entry corresponding to the file is freed. 
 * This function returns PFE_OK if the operation is successful, an error condition otherwise.
 *Dev: Antoine
 */

/* 
 * Question : what to do with the free space in the PFftab, and length ? free 
 */

int PF_CloseFile (int fd) {
	PFftab_ele* pt;
	int unixfd, error, ret;

	pt = (PFftab + sizeof(PFftab_ele)*fd);
	unixfd = pt->unixfd; /* to close the file */

	if(pt->hdrchanged=TRUE){
		/* TODO */
		/* write to the file header the new number of pages (pt->hdr.numepages) (cast integer to char) */
	}

	/* check if all pages are unpinned before deleting it : already managed into flushbuf */
	ret = BF_FlushBuf(fd);
	if(ret != BFE_OK){
	 return ret;
	}


	if ((error = close(unixfd)) < 0) {
		printf("close failed for file '%s'", pt->fname);
		return PFE_UNIX;
    }
    printf("\nThe file '%s' containing %d pages has been closed.\n", pt->fname, pt->hdr.numpages);
	return BFE_OK;
}





/* 
 *    int     fd;          PF file descriptor
 *    int     *pageNum;    return the number of the page allocated
 *    char    **pagebuf;   return a pointer to the page content
 * This function allocates a page in the file associated with a file descriptor fd. This new page
 * is appended to the end of the file. This function also allocates a buffer entry corresponding to
 * the new page. The value of pageNum for the page being allocated must be determined from the
 * information stored in the file header. The page allocated by this function is pinned and marked
 * dirty so that it will be written to the file eventually. Upon a successful page allocation, 
 * the file header must be updated accordingly. This function returns PFE_OK if the operation is
 * successful, an error condition otherwise.
 * Dev: Patric
 */
int PF_AllocPage (int fd, int *pagenum, char **pagebuf) {
	/*&pagenum = ++PFftab[fd]->hdr->numpages; *//* allocate a new page in given file */
	/*PFftab[fd]->hdrchanged = TRUE;*/

	 
}


/*
 *Get the page after pagenum and return *pagenum+1
 *Dev: Paul
 */

int PF_GetNextPage (int fd, int *pageNum, char **pagebuf){
	int ret;  
	PFftab_ele* pt;
		
	/*parameters for calling BF_Layer*/
	PFpage* fpage;
	BFreq bq;
	
	
	/* fill bq using PFftab_ele fields */
	
	pt=(PFftab+sizeof(PFftab_ele)*fd);
	bq.fd=fd;
	bq.unixfd=pt->unixfd;
	bq.dirty=FALSE;
	bq.pagenum=(*pageNum)+1;
	
	if( pt->hdr.numpages < (*pageNum)+1) return PFE_EOF; /* the wanted page does not exist */
	
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
	
	pt=(PFftab+sizeof(PFftab_ele)*fd);
	bq.fd=fd;
	bq.unixfd=pt->unixfd;
	bq.dirty=FALSE;
	bq.pagenum=pageNum;
	
	if( pt->hdr.numpages < pageNum) return PFE_EOF; /* the wanted page does not exist */
	
	ret=BF_GetBuf(bq,&fpage);
	if(ret!=0) BF_ErrorHandler(ret);
	
	*pagebuf= (fpage->pagebuf);
	
	
	return PFE_OK;


}


/*
 * After checking the validity of the fd and pageNum values, this function marks the page associated with pageNum and fd dirty. 
 * It returns PFE_OK if the operation is successful, an error condition otherwise.
 *Dev: Antoine 
 */
int PF_DirtyPage(int fd, int pageNum){

	return PFE_OK;
}




/* 
 *After checking the validity of the fd and pageNum values, this function unpins the page numbered pageNum of the file with file descriptor     *fd.  Besides, if the argument dirty is TRUE, the page is marked dirty. This function returns PFE_OK if the operation is successful, an error   *condition otherwise.
 * 
 *Dev Patric
 */

int PF_UnpinPage(int fd, int pageNum, int dirty){}

    
 /*
  * Used in pftest.c
  */

void PF_PrintError (char *error){}




