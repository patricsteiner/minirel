/* 
 * If the program is given no command-line arguments, it runs all tests.
 * You can run only some of the tests by giving the numbers of the tests
 * you want to run as command-line arguments.  They can appear in any order.
 * Notice that the clean up function is the last one, and it has associated
 * to it the number (number of tests + 1).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "minirel.h"
#include "pf.h"
#include "hf.h"
#include "am.h"

#define FILE1       "testrel"
#define STRSIZE     32
#define TOTALTESTS  3

/* prototypes for all of the test functions */

void amtest1(void);
void amtest2(void);
void amtest3(void);
void cleanup(void);

/* array of pointers to all of the test functions (used by main) */

void (*tests[])() = {amtest1, amtest2, amtest3, cleanup};


/**********************************************************/
/* amtest1:                                               */
/* Inserts records (which are just string of characters)  */
/* in an HF file, and also in the B+ Tree created for the */  
/* file .                                                 */
/**********************************************************/
void amtest1()
{
   int value,id, am_fd, hf_fd;
   char string_val[STRSIZE];
   char files_to_delete[80];
   RECID recid;

   /* To avoid uninitialized bytes error reported by valgrind */
   /* Bongki, Mar/13/2011 */
   memset(string_val,'\0',STRSIZE);

   printf("***** start amtest1 *****\n");
   /* making sure FILE1 index is not present */
   sprintf(files_to_delete, "rm -f %s*", FILE1);
   system(files_to_delete);

   /* Creating HF file to insert records */
   if (HF_CreateFile(FILE1, STRSIZE) != HFE_OK) {
      HF_PrintError("Problem creating HF file");
      exit(1);
   }

   /* Opening HF file so that below we can insert records inmediately */
   if ((hf_fd = HF_OpenFile(FILE1)) < 0) {
      HF_PrintError("Problem opening");
      exit(1);
   }

   if (AM_CreateIndex(FILE1, 1, STRING_TYPE, STRSIZE, FALSE) != AME_OK) {
      AM_PrintError("Problem creating");
      exit(1);
   }
   if ((am_fd = AM_OpenIndex(FILE1,1)) < 0) {
      AM_PrintError("Problem opening");
      exit(1);
   }

   /* Inserting value in the HF file and the B+ Tree */
      value = 10;
      while (value < 100)
      {
         sprintf(string_val, "entry%d", value);
         /* Notice the recid value being inserted is trash.    */
         /* The (char *)& is unnecessary in AM_InsertEntry     */
         /* because in this case string_val is (char *)        */
         /* but I am putting it here to emphasize what to pass */
         /* in other cases (int and float value                */

         /* Inserting the record in the HF file */
         recid = HF_InsertRec(hf_fd, string_val);
         if (!HF_ValidRecId(hf_fd,recid)){
            HF_PrintError("Problem inserting record in HF file");
            exit(1);
         }
     
         /* Inserting the record in the B+ Tree */
         if (AM_InsertEntry(am_fd, (char *)&string_val, recid) != AME_OK) {
             AM_PrintError("Problem Inserting rec");
             exit(1);
         }
          value = value + 2;
      }

   if (HF_CloseFile(hf_fd) != AME_OK){
      HF_PrintError("Problem closing file");
      exit(1);
   }

   if (AM_CloseIndex(am_fd) != AME_OK) {
      AM_PrintError("Problem Closing");
      exit(1);
   }

   printf("***** end amtest1 *****\n");

}


/**************************************************/
/* amtest2:                                       */
/* Retrieves values from the file created using   */
/* amtest1.  A scan is used for the retrieval     */
/**************************************************/
void amtest2()
{
   int am_fd, hf_fd;
   int sd;
   RECID recid;
   char comp_value[STRSIZE];
   char retrieved_value[STRSIZE];

   printf("***** Start amtest2 *****\n");
   /* using amtest1() to generate file and index */
   amtest1();

   if ((hf_fd = HF_OpenFile(FILE1)) < 0) {
      HF_PrintError("Problem opening");
      exit(1);
   }
   if ((am_fd = AM_OpenIndex(FILE1,1)) < 0) {
      AM_PrintError("Problem opening index");
      exit(1);
   }

   /* Initializing the compare value */
   memset(comp_value, ' ', STRSIZE);
   sprintf(comp_value, "entry50");

   /* Opening the index scan */
   if ((sd = AM_OpenIndexScan(am_fd, GT_OP, comp_value)) < 0) {
      AM_PrintError("Problem opening index scan");
      exit(1);
   }

   /* Pulling recid using the B+ Tree and printing the record */
   /* by using the retrieved recid and HF_GetThisRec()        */

   while (1)
   {
      /* clearing retrieved_value  */
      memset(retrieved_value, ' ', STRSIZE);
      recid = AM_FindNextEntry(sd);
      if (!HF_ValidRecId(hf_fd,recid)) 
         if (AMerrno == AME_EOF) break; /*Out of records satisfying predicate */
      else
      {
         AM_PrintError("Problem finding next entry");
         exit(1);
      }
      if (HF_GetThisRec(hf_fd, recid, retrieved_value) != HFE_OK){
         HF_PrintError("Problem retrieving record");
         exit(1);
      } 
      printf("the retrieved value is %s\n", retrieved_value);
   }

   /* closing scan, index file, and HF file */
   if (AM_CloseIndexScan(sd) != AME_OK) {
      AM_PrintError("Problem closing index scan");
      exit(1);
   }

   if (AM_CloseIndex(am_fd) != AME_OK) {
      AM_PrintError("Problem closing index file");
      exit(1);
   }

   if (HF_CloseFile(hf_fd) != HFE_OK) {
      HF_PrintError("Problem closing HF file");
      exit(1);
   }

   printf("***** end amtest2 *****\n");
}

/*******************************************************/
/* amtest3:                                            */
/* The same as amtest2, but now we are doing B+ Tree   */
/* entry deletions during an index scan.               */
/*******************************************************/
void amtest3()
{
   int am_fd, hf_fd;
   int sd;
   RECID recid;
   char comp_value[STRSIZE];
   char retrieved_value[STRSIZE];

   printf("***** Start amtest3 *****\n");
   /* using amtest1() to generate file and index */
   amtest1();

   if ((hf_fd = HF_OpenFile(FILE1)) < 0) {
      HF_PrintError("Problem opening");
      exit(1);
   }
   if ((am_fd = AM_OpenIndex(FILE1,1)) < 0) {
      AM_PrintError("Problem opening index");
      exit(1);
   }

   /* Initializing the compare value */
   memset(comp_value, ' ', STRSIZE);
   sprintf(comp_value, "entry50");

   /* Opening the index scan */
   if ((sd = AM_OpenIndexScan(am_fd, GT_OP, comp_value)) < 0) {
      AM_PrintError("Problem opening index scan");
      exit(1);
   }

   /* Pulling recid using the B+ Tree and printing the record */
   /* by using the retrieved recid and HF_GetThisRec()        */

   while (1)
   {
      /* clearing retrieved_value  */
      memset(retrieved_value, ' ', STRSIZE);
      recid = AM_FindNextEntry(sd);
      if (!HF_ValidRecId(hf_fd,recid)) 
         if (AMerrno == AME_EOF) break; /*Out of records satisfying predicate */
      else
      {
         AM_PrintError("Problem finding next entry");
         exit(1);
      }
      if (HF_GetThisRec(hf_fd, recid, retrieved_value) != HFE_OK){
         HF_PrintError("Problem retrieving record");
         exit(1);
      } 
      printf("the retrieved value is %s\n", retrieved_value);
      /* We are doing a deletion during an index scan            */
      /* We are deleting those entries from the B+Tree which     */
      /* are less that "entry70". Notice we are NOT deleting the */
      /* entry from the HF file.                                 */ 
      
      if (strcmp(retrieved_value, "entry70") < 0){
         if (AM_DeleteEntry(am_fd, retrieved_value, recid) != AME_OK) {
            AM_PrintError("Problem deleting entry");
            exit(1);
         }
         else 
            printf("DELETING entry %s\n", retrieved_value);
      }

   } /* while end */

   /* closing scan, index file, and HF file */
   if (AM_CloseIndexScan(sd) != AME_OK) {
      AM_PrintError("Problem closing index scan");
      exit(1);
   }

   if (AM_CloseIndex(am_fd) != AME_OK) {
      AM_PrintError("Problem closing index file");
      exit(1);
   }

   if (HF_CloseFile(hf_fd) != HFE_OK) {
      HF_PrintError("Problem closing HF file");
      exit(1);
   }

   printf("***** end amtest3 *****\n"); 
}

/*********************************************/
/* cleanup:                                  */
/* Gets rid of files generated by the tests  */
/*********************************************/
void cleanup(void)
{
   char files_to_delete[50];

   sprintf(files_to_delete, "rm -f %s*", FILE1);
   system(files_to_delete);
} 

/****************/
/* main program */
/****************/

int main(int argc, char *argv[])
{
  char *progname = argv[0];	/* since we will be changing argv */
  int testnum;
  
  
  /*****************************************/
  /*  IMPORTANT INITIALIZATION             */
  /*****************************************/
  AM_Init(); /* Initializes also the HF_Init */
  
  /* if no arguments given, do all tests */
  if(argc == 1){
    for(testnum = 0; testnum < TOTALTESTS; ++testnum)
      if(tests[testnum] != NULL)
	(tests[testnum])();
  }
  
  /* otherwise, perform specified tests */
  else{
    
    /* for each command-line argument... */
    while(*++argv != NULL){
      
      /* make sure it's a number */
      if(sscanf(*argv, "%d", &testnum) != 1){
	fprintf(stderr, "%s: %s is not a number\n", progname, *argv);
	continue;
      }
      
      /* make sure it's in range */
      if(testnum < 1 || testnum > TOTALTESTS + 1){
	printf("Valid test numbers are between 1 and %d\n", TOTALTESTS + 1);
	continue;
      }
      
      /* perform the requested test */
      (tests[testnum - 1])();
    }
  }
  
  exit(0);
}

