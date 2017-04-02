#include <stdio.h>
#include "bf.h"
#include "minirel.h"
#include "pfHeader.h"


PFftab_ele *PFftab; 
int PFftab_length;


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
 * 
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


	















