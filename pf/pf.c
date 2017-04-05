#include <stdio.h>
#include <unistd.h>
#include "bf.h"
#include "minirel.h"
#include "pfHeader.h"


PFftab_ele *PFftab; 
size_t PFftab_length;



/*
 * Init the PF layer and also the BF layer by BF_Init
 * Creates and initialized the PFftab: file table of PFfile
 * Dev : Paul
 */
void PF_Init(void){
	PFftab=malloc(sizeof(PFftab_ele)*PF_FTAB_SIZE); //PFftab is an array of PF_FTAB_SIZE PFf_tabe_ele elements. 
	PFftab_length=0; // the file table is still empty;
	BF_Init();
	
	
}

/*
 * Creates File and then create pffile table entry, writes the header of the file and close it.
 * Dev : Paul
 */



int PF_CreateFile(char *filename){
	int unixfd=0;
	int PFfd=0;
	ino_t inode;
	PFftab_ele* pt=(PFftab+sizeof(PFftab_ele)*PFftab_length);
	
	fd=open(filename, O_CREAT,O_RDWR);
	
	if (fd < 0) {  
	    // problem occurred while opening the file
	    perror("Error opening the file");
	}   
        /* inode number is necessary to create PFftab entry */

	struct stat file_stat;  
	int ret;  
	ret = fstat(fd, &file_stat);  
	inode = file_stat.st_ino; 
	
	/*fill the next array of the PF file table*/
	pt->valid=TRUE;
	pt->inode=inode;
	snprintf(pt->fname,  sizeof(filename),"%s", filename);/////////////////////////////////à tester///////////////////
	pt->unixfd=unixfd;
	pt->hdr->numpages=0;
	pt->hdrchanged=false; 
	
	lengh
	
	/* hdrchanged is false, because next step is the copy of the header in the first page of the file*/

}

/*
 * This function destroys the file filename. The file should have existed, and should not be already 
 * open. This function returns PFE_OK if the operation is successful, an error condition otherwise.
 * Dev: Patric
 */
int PF_DestroyFile (char *filename) {
	if (access(filename, F_OK) != -1) { // if file exists
		return (unlink(filename));
	}
	return -1; // TODO better error code?
}

/*
 *
 *Dev: Antoine
 */

int PF_OpenFile (char *filename)

/*
 *
 *Dev: Antoine
 */

int PF_CloseFile (int fd)





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
	&pagenum = ++PFftab[fd]->hdr->numpages; /* allocate a new page in given file */
	PFftab[fd]->hdrchanged = TRUE;
	 
}


/*
 *
 *Dev: Paul
 */

int PF_GetNextPage (int fd, int *pageNum, char **pagebuf){}

/*
 *
 *Dev: Paul
 */


int PF_GetFirstPage (int fd, int *pageNum, char **pagebuf){}

/*
 *
 *Dev: Paul 
 */


int PF_GetThisPage (int fd, int *pageNum, char **pagebuf){}


/*
 *
 *Dev: Antoine 
 */
int PF_DirtyPage(int fd, int pageNum){}




/* 
 *After checking the validity of the fd and pageNum values, this function unpins the page numbered pageNum of the file with file descriptor     *fd.  Besides, if the argument dirty is TRUE, the page is marked dirty. This function returns PFE_OK if the operation is successful, an error   *condition otherwise.
 * 
 *Dev Patric
 */

int PF_UnpinPage(int fd, int pageNum, int dirty){}

    
 






