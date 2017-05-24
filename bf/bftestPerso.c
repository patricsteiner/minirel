#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "minirel.h"
#include "bf.h"

#define FILE_CREATE_MASK (S_IRUSR|S_IWUSR|S_IRGRP)

#define FILE1 "file1"
#define FD1 10

#define FILE2 "file2"
#define FD2 20

static BFreq breq;
char header[PAGE_SIZE];


void writefiles(char *fname1, char *fname2){
    PFpage *fpage;
    int i;
    int fd1,unixfd1,pagenum1;
    int fd2,unixfd2,pagenum2;
    int error;

    printf("\n******** %s opened for write ***********\n",fname1);
    printf("\n******** %s opened for write ***********\n\n",fname2);

    /* open file1 */
    if ((unixfd1 = open(fname1, O_RDWR|O_CREAT, FILE_CREATE_MASK))<0){
	printf("open failed: file1");
	exit(-1);
    }

    /* open file2 */
    if ((unixfd2 = open(fname2, O_RDWR|O_CREAT, FILE_CREATE_MASK))<0){
	printf("open failed: file2");
	exit(-1);
    }

      /* write an empty page header for both files*/
    memset(header, 0x00, PAGE_SIZE);
    if(write(unixfd1, header, PAGE_SIZE) != PAGE_SIZE) {
	fprintf(stderr,"writefile writing header failed: %s\n",fname1);
	exit(-1);
    }

    if(write(unixfd2, header, PAGE_SIZE) != PAGE_SIZE) {
	fprintf(stderr,"writefile writing header failed: %s\n",fname2);
	exit(-1);
    }

	/* Allocate BF_MAX_BUFS pages for each files alternatively*/ 
	for (i=0; i < 4 * BF_MAX_BUFS; i++){
		if(i%2 == 0){
			breq.fd = FD1;
			breq.unixfd = unixfd1;
        	breq.pagenum = i/2;
		}else{
			breq.fd = FD2;
			breq.unixfd = unixfd2;
			breq.pagenum = (i-1)/2;
		}
        /* allocate a page for file 1*/
        if((error = BF_AllocBuf(breq, &fpage)) != BFE_OK) {
	    	printf("alloc buffer failed");
	    	exit(-11);
		}
		sprintf((char*)fpage,"%4d%4d",breq.fd,breq.pagenum);
		printf("allocated page: fd=%d, pagenum=%d\n",breq.fd,breq.pagenum);
		/* mark this page dirty */
		if(BF_TouchBuf(breq) != BFE_OK){
		    printf("touching buffer error, %d\n",i);
		    exit(-1);
		}
		/* unpin this page */
		if ((error = BF_UnpinBuf(breq))!= BFE_OK){
		    printf("unpin buffer failed");
		    exit(-11);
		}
	}


	BF_ShowBuf();

	/* Flush all buffer pages for this file */
	breq.fd = FD2;
	printf("\nFlushing file1\n");
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    BF_ShowBuf();

    breq.fd = FD1;
    printf("\nFlushing file1\n");
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    printf("\n ********** written file being closed **********\n");

   	BF_ShowBuf();

	/* close the files */
    if ((error = close(unixfd1)) < 0) {
	printf("close failed : file1");
	exit(-1);
    }
    if ((error = close(unixfd2)) < 0) {
	printf("close failed : file2");
	exit(-1);
    }

}

void readfiles(char *fname1, char *fname2){
	int i, error;
	int unixfd1, fd1, pagenum1;
	int unixfd2, fd2, pagenum2;

	int fd, pagenum;
	PFpage *fpage;

	/* Opening files */
	if ((unixfd1 = open(fname1, O_RDWR))<0){
		printf("open failed: %s",fname1);
		exit(-1);
    }
    if ((unixfd2 = open(fname2, O_RDWR))<0){
		printf("open failed: %s",fname2);
		exit(-1);
    }

    printf("\n ********* reading files alternatively via buffer **********\n");


    for (i=0; i < 4 * BF_MAX_BUFS; i++){
    	if(i%2 == 0){
		    breq.fd = FD1;
		    breq.unixfd = unixfd1;
        	breq.pagenum = i/2;

        	if((error = BF_GetBuf(breq, &fpage)) != BFE_OK) {
				printf("getBuf failed: %s, %d",fname1,i);
				exit(-1);
			}

			sscanf((char*)fpage,"%4d%4d",&fd,&pagenum);
			printf("values from buffered page %d: %d %d\n",i,fd,pagenum);
			fflush(stdout);

			/* unpin this page */
			if ((error = BF_UnpinBuf(breq))!= BFE_OK){
	    		printf("unpin buffer failed");
	    		exit(-11);
			}

    	}else{
    		breq.fd = FD2;
		    breq.unixfd = unixfd2;
        	breq.pagenum = (i-1)/2;

        	if((error = BF_GetBuf(breq, &fpage)) != BFE_OK) {
				printf("getBuf failed: %s, %d",fname2,i);
				exit(-1);
			}

			sscanf((char*)fpage,"%4d%4d",&fd,&pagenum);
			printf("values from buffered page %d: %d %d\n",i,fd,pagenum);
			fflush(stdout);

			/* unpin this page */
			if ((error = BF_UnpinBuf(breq))!= BFE_OK){
	    		printf("unpin buffer failed");
	    		exit(-11);
			}
    	}
    }


    BF_ShowBuf();

    /* Flush all buffer pages for each file */
   
    breq.fd = FD1;
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    BF_ShowBuf();

    breq.fd = FD2;
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    BF_ShowBuf();
    

    printf("\n ********** eof reached **********\n");


	/* close the files */
    if ((error = close(unixfd1)) < 0) {
	printf("close failed : file1");
	exit(-1);
    }
    if ((error = close(unixfd2)) < 0) {
	printf("close failed : file2");
	exit(-1);
    }
}


void printfile(char *fname){
	int i, unixfd, nbytes, error;
	int fd, pagenum;
	PFpage fpage;

	printf("\n ***** printing file : %s *****\n", fname);

	if ((unixfd = open(fname, O_RDONLY))<0){
		printf("open failed: %s",fname);
		exit(-1);
    }

    /* read the file page per page */ 
    for(i=0; (nbytes = read(unixfd, &fpage, sizeof(PFpage))) > 0 ;i++){
    	if( nbytes!= sizeof(PFpage)) {
    		printf("read failed: %s, %d\n",fname, i);
    		exit(-1);
    	}

    	sscanf((char*)&fpage,"%4d%4d",&fd,&pagenum);
    	printf("values from disk page %d: %d %d\n",i,fd,pagenum);
    	fflush(stdout);
    }

    printf("\n ********** eof reached **********\n");

    /* close the file */
    if ((error = close(unixfd)) < 0) {
		printf("close failed : file1");
		exit(-1);
    }
}

void testbf2(void){
	char        command[128];
	unlink(FILE1);
	unlink(FILE2);

	/* write files alternatively */
	writefiles(FILE1, FILE2);
	fflush(stdout);

	printf("\n ****** Showing the files has been written *****\n");
    fflush(stdout);
    sprintf(command, "ls -al %s %s", FILE1, FILE2);
    system(command);
    fflush(stdout);

    /* print it out */
    printfile(FILE1);
    fflush(stdout);
    printfile(FILE2);
    fflush(stdout);

    /* read it out */
    readfiles(FILE1, FILE2);
    fflush(stdout);

   	printf("\n ****** Showing the file has been written *****\n");
    fflush(stdout);
    sprintf(command, "ls -al %s %s", FILE1, FILE2);
    system(command);
    fflush(stdout);


}

int main(void){

	PFpage *fpage;
	printf("**** Tests bf2 ******\n");

	BF_Init();
	BF_ShowBuf();

	testbf2();


	return 0;
}