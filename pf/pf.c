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
#include "pfUtils.h"


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
	pt=(PFftab+sizeof(PFftab_ele)*PFftab_length);

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
	size_t i;
	PFftab_ele* ftab_ele;
	ftab_ele = NULL;
	/* search for file in PFftab by filename*/
	for (i = 0; i < PFftab_length; i++) {
		if (strcmp(PFftab[i].fname, filename) == 0) {
			ftab_ele = &PFftab[i]; /* found the page */
			break;
		}
	}
	if (ftab_ele == NULL) { /* if file not in PFftab */
		return PFE_FILENOTINTAB;
	}
	else if (ftab_ele->valid == TRUE) { /* if file open */
		return PFE_FILEOPEN;
	}
	else if (access(filename, F_OK) == -1) { /* if file does not exist */
		return PFE_FILENOTEXISTS;
	}
	else return unlink(filename); /* delete it */
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

   	/* prepare buffer request */
    bq.unixfd = unixfd;
    bq.fd = PFftab_length; 
    bq.pagenum = 0; /*header is the first page of a file */
    bq.dirty = FALSE;

    res = BF_GetBuf(bq, &fpageHeader); /* PIN of the file is set to 1 */
    if(res != BFE_OK) BF_ErrorHandler(res);

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
    sprintf(pt->fname,"%s",filename);
    pt->unixfd = unixfd;
    memcpy((int*) &(pt->hdr.numpages), (char*)fpageHeader->pagebuf, sizeof(int));
    /*pt->hdr.numpages = atoi(fpageHeader->pagebuf);*/
    pt->hdrchanged = FALSE;

    PFftab_length++; /* for next entry */

    res= BF_UnpinBuf(bq); /* page has pinned to 0 now */
	if(res!=0) BF_ErrorHandler(res);

    printf("\nThe file '%s' containing %d pages has been added to the PFtable.\n", filename, pt->hdr.numpages);
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
	pt = (PFftab + sizeof(PFftab_ele) * fd);

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
    	if(error!=BFE_OK) return error;

    	/* Unpin the page after */
    	error = BF_UnpinBuf(bq);
    	if (error != BFE_OK) return error;
	}

	/* Flush buffer before closing the file */
	error = BF_FlushBuf(fd);
	if(error != BFE_OK) return error;

	/* Close the file with unix close*/
	if (close(pt->unixfd) < 0) {
		printf("close failed for file '%s'", pt->fname);
		return PFE_UNIX;
    }

    pt->valid = FALSE;
    PFftab_length--;
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
	int new_pagenum;
	BFreq bq;
	PFpage* pfpage;
	int error;

	if (fd < 0 || fd >= PFftab_length) {
		return PFE_FD;
	}
	/* TODO: appedned to end of the file? ...how to do that */
	
	new_pagenum = PFftab[fd].hdr.numpages;	
	/*alternate? way to get pfftab_ele = (PFftab + sizeof(PFftab_ele) * fd);*/
	bq.fd = fd;
	bq.unixfd = PFftab[fd].unixfd;
	bq.dirty = TRUE;
	bq.pagenum = new_pagenum;
	/*pfpage = malloc(sizeof(PFpage));*/
	if ((error = BF_AllocBuf(bq, &pfpage)) != BFE_OK) {
		BF_ErrorHandler(error);
	}
	/* make page dirty */
	if ((error = BF_TouchBuf(bq)) != BFE_OK) {
		BF_ErrorHandler(error);
	}
	/* update page info */
	*pagenum = new_pagenum;
	*(pagebuf)=pfpage->pagebuf;
	PFftab[fd].hdrchanged = TRUE;

	PFftab[fd].hdr.numpages++ ;

	/*print lru buffer*/
	return PFE_OK;
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
	
	pt=(PFftab+sizeof(PFftab_ele)*fd);
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
	
	ftab_ele = (PFftab + sizeof(PFftab_ele) * fd);

	if (pageNum < 0 || pageNum >= ftab_ele->hdr.numpages){
		printf("\nnumpages : %d\n", ftab_ele->hdr.numpages);
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
	
	pfftab_ele = &PFftab[fd];
	/*alternate way to get pfftab_ele = (PFftab + sizeof(PFftab_ele) * fd);*/
	if (pageNum < 0 || pageNum >= pfftab_ele->hdr.numpages) {
		return PFE_INVALIDPAGE;
	}
	
	bq.fd = fd;
	bq.unixfd = pfftab_ele->unixfd;
	bq.dirty = dirty;
	bq.pagenum = pageNum;
	
	if ((error = BF_UnpinBuf(bq)) != BFE_OK) {
		BF_ErrorHandler(error);
	}
	if (dirty) { /* mark page as dirty when it should be dirty */
		if ((error = BF_TouchBuf(bq)) != BFE_OK) {
			BF_ErrorHandler(error);
		}
	}
	return PFE_OK;
}

    
void PF_PrintError (char *error){
	printf("\nPF_PrintError : %s\n", error);
}