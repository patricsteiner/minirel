#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "minirel.h"
#include "bf.h"
#include "buf.h"


/*
 * default files
 */
#define FILE1	"file1"
#define FD1	10

static BFreq breq;

/*
 * Open the file, allocate as many pages in the file as the buffer manager
 * would allow, and write the page number into the data, then close file.
 */
void writefile(char *fname)
{
    PFpage *fpage;
    int i;
    int fd,unixfd,pagenum;
    int error;

    /* open file1 */
    if ((unixfd = open(fname, O_RDWR|O_CREAT))<0){
	printf("open failed: file1");
	exit(-1);
    }

    printf("\n******** %s opened for write ***********\n",fname);

    breq.fd = FD1;
    breq.unixfd = unixfd;

    for (i=0; i < 2 * BF_MAX_BUFS; i++){
        breq.pagenum = i;

        /* allocate a page */
        if((error = BF_AllocBuf(breq, &fpage)) != BFE_OK) {
	    printf("alloc buffer failed");
	    exit(-11);
	}

	/* memcpy(&fpage, (char *)&i, sizeof(int)); */
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
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    printf("\n ********** written file being closed **********\n");

    BF_ShowBuf();

    /* close the file */
    if ((error = close(unixfd)) < 0) {
	printf("close failed : file1");
	exit(-1);
    }
}

/*
 * read the contents off the specified file via the buffer manager
 */
void readfile(char *fname)
{
    int i, unixfd, error;
    int fd, pagenum;
    PFpage *fpage;


    if ((unixfd = open(fname, O_RDWR))<0){
	printf("open failed: %s",fname);
	exit(-1);
    }

    printf("\n ********* reading file via buffer **********\n");

    breq.fd = FD1;
    breq.unixfd = unixfd;

    for (i=0; i < 2 * BF_MAX_BUFS; i++){
        breq.pagenum = i;

	if((error = BF_GetBuf(breq, &fpage)) != BFE_OK) {
		printf("getBuf failed: %s, %d",fname,i);
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

    BF_ShowBuf();

    /* Flush all buffer pages for this file */
    if((error = BF_FlushBuf(breq.fd)) != BFE_OK) {
	    printf("flush buffer failed");
	    exit(-12);
    }

    printf("\n ********** eof reached **********\n");

    BF_ShowBuf();

    /* close the file */
    if ((error = close(unixfd)) < 0) {
	printf("close failed : file1");
	exit(-1);
    }
}

/*
 * print the contents off the specified file
 */
void printfile(char *fname)
{
    int i, unixfd, nbytes, error;
    int fd, pagenum;
    PFpage fpage;

    printf("\n ********* printing file **********\n");

    if ((unixfd = open(fname, O_RDONLY))<0){
	printf("open failed: %s",fname);
	exit(-1);
    }

    for(i=0; (nbytes=read(unixfd,&fpage,sizeof(PFpage)))>0 ;i++) {
	if (nbytes != sizeof(PFpage)) {
		printf("read failed: %s, %d",fname,i);
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

/*
 * general tests of PF layer
 */
void testpf1(void)
{
    int		i, error, pagenum;
    char*	buf;
    int		fd1, fd2;
    char        command[128];
    int temp;

    /* Making sure file don't exist */
    unlink(FILE1);

    /* write to file1 */
    writefile(FILE1);
    fflush(stdout);

    printf("\n ****** Showing the file has been written *****\n");
    fflush(stdout);
    sprintf(command, "ls -al %s", FILE1);
    system(command);
    fflush(stdout);

    /* print it out */
    printfile(FILE1);
    fflush(stdout);

    /* read it out */
    readfile(FILE1);
    fflush(stdout);

    printf("\n ****** Showing the file has been written *****\n");
    fflush(stdout);
    sprintf(command, "ls -al %s", FILE1);
    system(command);
    fflush(stdout);

    /* write to and read from file1 again */
/*
    writefile(FILE1);
    printfile(FILE1);

    printf("\n ****** Showing the file has been written *****\n");
    sprintf(command, "ls -al %s", FILE1);
    system(command);
*/
}

main()
{
  /* initialize PF layer */
  BF_Init();

  printf("\n************* Starting testpf1 *************\n");
  testpf1(); 
  printf("\n************* End testpf1 ******************\n");
}
