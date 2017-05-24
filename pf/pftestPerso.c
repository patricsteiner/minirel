#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "minirel.h"
#include "bf.h"
#include "pf.h"


#define FILE1	"file1"
#define FILE2	"file2"

void writefiles(char *fname1, char *fname2){
	int i, j, error;
	int fd1, fd2;
	int fd, pagenum;
	char *buf;

	PF_PrintTable();
	/* open file1, and allocate a few pages in there */
    if ((fd2=PF_OpenFile(fname2))<0){
        printf("\n%d\n", fd2);
		PF_PrintError("open file2");
		exit(1);
    }
    PF_PrintTable();
    /* open file2, and allocate a few pages in there */
    if ((fd1=PF_OpenFile(fname1))<0){
        printf("\n%d\n", fd1);
		PF_PrintError("open file1");
		exit(1);
    }
    PF_PrintTable();

    printf("\n******** %s opened for write ***********\n",fname1);
    printf("\n******** %s opened for write ***********\n",fname2);


    for (i=0; i < 2 * BF_MAX_BUFS; i++){
    	if ((error = PF_AllocPage(fd1,&pagenum,&buf))!= PFE_OK){
            printf("PF_AllocPage fails (i=%d)\n",i);
    	    PF_PrintError("first buffer\n");
    	   	exit(1);
    	}

    	memcpy(buf, (char *)&i, sizeof(int));

    	printf("allocated page %d, value_written %d\n",pagenum,i);

    	/* mark all these pages dirty */
    	if(error = PF_DirtyPage(fd1, pagenum) != PFE_OK){
    		printf("%d",error); 
    	    PF_PrintError("PF_DirtyPage");
    	    exit(1);
    	}

    	/* unfix these pages */
    	if ((error = PF_UnpinPage(fd1, pagenum,FALSE))!= PFE_OK){
            printf("\n%d\n", error );
    	    PF_PrintError("unfix buffer");
    	    exit(1);
    	}

    }


    /* close the files */
    if ((error = PF_CloseFile(fd1))!= PFE_OK){
	   	PF_PrintError("close file1");
       	printf("%d\n", error);
		exit(1);
    }
    if ((error = PF_CloseFile(fd2))!= PFE_OK){
	   	PF_PrintError("close file2");
       	printf("%d\n", error);
		exit(1);
    }
	if ((error = PF_DestroyFile(FILE2))!= PFE_OK){
		printf("héhéhéh %d\n", error);
	   	
		PF_PrintError("close file2");
       	
		exit(1);
    }
	 if ((error = PF_DestroyFile(FILE1))!= PFE_OK){
	   	PF_PrintError("close file2");
       	printf("%d\n", error);
		exit(1);
    }

}



void testpf2(void){
	int error;

	printf("unlink");
    unlink(FILE1);
    unlink(FILE2);

    /* create files */
    if ((error = PF_CreateFile(FILE1)) != PFE_OK){
		PF_PrintError(FILE1);
		exit(1);
    }
    printf("\n*********** %s created *********\n", FILE1);

    if ((error = PF_CreateFile(FILE2)) != PFE_OK){
		PF_PrintError(FILE2);
		exit(1);
    }
    printf("\n*********** %s created *********\n", FILE2);

    /* write to files */
    writefiles(FILE1, FILE2);


}

int main(void){
	PF_Init();
	testpf2();
	return 0;
}
