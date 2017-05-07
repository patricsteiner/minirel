#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "minirel.h"
#include "hf.h"

#define RECSIZE 80
#define STRSIZE 80
#define NUMBER  100
#define RECORDVAL 77
#define FILE1 "recfile"
#define FILE2 "compfile"

#ifndef offsetof
#define offsetof(type, field)   ((size_t)&(((type *)0) -> field))
#endif

struct rec_struct
{
   char string_val[STRSIZE];
   float float_val;
   int int_val;
};


/****************************************************************/
/* hftest1:                                                     */
/* Insert NUMBER records and delete those that are odd numbered.*/
/* Note that deleted records are still in the file, and simply  */
/* marked as deleted.                                           */
/****************************************************************/

void hftest1()
{

  int i,fd; 
  RECID recid;
  RECID next_recid;
  char recbuf[RECSIZE];
  char record[RECSIZE];

  /* making sure FILE1 doesn't exist */
  unlink(FILE1);

  /* Creating and opening the HF file */

  if (HF_CreateFile(FILE1,RECSIZE) != HFE_OK)
     HF_PrintError("Problem creating HF file.\n");
  if ((fd = HF_OpenFile(FILE1)) < 0)
     HF_PrintError("Problem opening HF file.\n");

  /* Adding the records */
    
  for (i = 0; i < NUMBER; i++)
  {
     memset(recbuf,' ',RECSIZE);
     sprintf(recbuf, "record%d", i);
     printf("adding record: %s\n", recbuf);
     recid = HF_InsertRec(fd, recbuf);
     if (!HF_ValidRecId(fd,recid))
     {
        HF_PrintError("Problem inserting record.\n");
        exit(1);
     }
  }   

  /* Getting the records */

  next_recid = HF_GetFirstRec(fd, record);
  if (!HF_ValidRecId(fd,next_recid))
  {
      HF_PrintError("Problem getting first record.\n");
      exit(1);
  }
   
  i = 0;
  while (HF_ValidRecId(fd,next_recid))
  {
    /* delete record if 'i' is odd numbered */
    if ((i % 2) != 0) 
    {
       printf("deleting record: %s\n", record);
       if (HF_DeleteRec(fd, next_recid) != HFE_OK){
          HF_PrintError("Problem deleting record.\n");
          exit(1);
       }
    }
    recid = HF_GetNextRec(fd, next_recid, record);
    next_recid = recid;
    i++;
  }
  if (HF_CloseFile(fd) != HFE_OK)
  {
    HF_PrintError("Problem closing file.\n");
    exit(1);
  }

  read_string_recs(FILE1);
}


/***************************************************/
/* hftest2:                                        */
/* Run hftest1 to insert and delete records.       */
/* Then, insert new records and show the space used*/
/* to store the old records previously is now      */
/* occupied by the new records.                    */
/***************************************************/

void hftest2()
{
   int i,fd;
   RECID recid;
   char record[RECSIZE];

   /* inserts and delete records in FILE1 */
   hftest1();

   if ((fd = HF_OpenFile(FILE1)) < 0) 
   {
      HF_PrintError("Problem opening FILE1\n"); 
      exit(1);
   }
    
   /* clearing up the record */
   memset(record, ' ', RECSIZE);

   /* actual insertion */
   for (i = 1; i < NUMBER; i++)
   {
      sprintf(record, "New record %d",i);
      printf("Inserting new record: %s\n", record);
      recid = HF_InsertRec(fd, record);
      if (!HF_ValidRecId(fd,recid))
      {
        HF_PrintError("Problem inserting record.\n");
        exit(1);
      }
   }

  if (HF_CloseFile(fd) != HFE_OK)
  {
    HF_PrintError("Problem closing file.\n");
    exit(1);
  }

  read_string_recs(FILE1);
}

/************************************************/
/* insert_struc_recs:                           */
/* Inserts structured records to a given file.  */
/************************************************/

int insert_struc_recs(char *filename)
{
  int i,fd;
  RECID recid;
  struct rec_struct record;

  /* making sure file doesn't exists */
  unlink(filename);

  /* Creating file with records of type rec_struct */  
  if (HF_CreateFile(filename, sizeof(struct rec_struct)) != HFE_OK) {
     HF_PrintError("Problem creating file.\n");
     exit(1);
  }
  
  /* Opening the file to get the file descriptor */
  if ((fd = HF_OpenFile(filename)) < 0) {
     HF_PrintError("Problem opening file\n.");
     exit(1);
  }
  
  /* Adding the records. */

  for (i = 0; i < NUMBER; i++) {
     memset((char *)&record, ' ', sizeof(struct rec_struct));
     sprintf(record.string_val, "entry%d", i);
     record.float_val = (float)i;
     record.int_val = i;
    
     printf("inserting structured record: (%s, %f, %d)\n",
		record.string_val, record.float_val, record.int_val);

     /* Actual insertion */
     recid = HF_InsertRec(fd, (char *)&record);
     if (!HF_ValidRecId(fd,recid)) {
        HF_PrintError("Problem inserting record.\n");
        exit(1);
     }
  }
  
  if (HF_CloseFile(fd) != HFE_OK) {
    HF_PrintError("Problem closing file.\n");
    exit(1);
  }
}

/**************************************************/
/* read_string_recs:                              */
/* reads string records from an input file.       */
/**************************************************/

int read_string_recs(char *filename)
{
  int fd;
  struct rec_struct record;
  RECID recid, next_recid;
  char recbuf[RECSIZE];
 
  /* opening the file */
  if ((fd = HF_OpenFile(filename)) < 0) {
     HF_PrintError("Problem opening file.\n");
     exit(1);
  }

  /* getting first record */
  recid = HF_GetFirstRec(fd, recbuf);
  if (!HF_ValidRecId(fd,recid)) {
     HF_PrintError("Problem getting record.\n");
     exit(1);
  }
  
  /* getting the rest of the records */
  while (HF_ValidRecId(fd,recid)) {
     /* printing the record value */
     printf("retrieved record: %s\n", recbuf);
     next_recid = HF_GetNextRec(fd, recid, recbuf);
     recid = next_recid;
  }

  if (HF_CloseFile(fd) != HFE_OK) {
     HF_PrintError("Problem closing file.\n");
     exit(1);
  }
}

/**************************************************/
/* read_struc_recs:                               */
/* reads structured records from an input file.   */
/**************************************************/

int read_struc_recs(char *filename)
{
  int fd;
  struct rec_struct record;
  RECID recid, next_recid;
  
 
  /* opening the file */
  if ((fd = HF_OpenFile(filename)) < 0) {
     HF_PrintError("Problem opening file.\n");
     exit(1);
  }

  /* getting first record */
  recid = HF_GetFirstRec(fd, (char *)&record);
  if (!HF_ValidRecId(fd,recid)) {
     HF_PrintError("Problem getting record.\n");
     exit(1);
  }
  
  /* getting the rest of the records */
  while (HF_ValidRecId(fd,recid)) {
     /* printing the record value */
     printf("retrieved structured record: (%s, %f, %d)\n",
		record.string_val, record.float_val, record.int_val); 
     next_recid = HF_GetNextRec(fd, recid, (char *)&record);
     recid = next_recid;
  }

  if (HF_CloseFile(fd) != HFE_OK) {
     HF_PrintError("Problem closing file.\n");
     exit(1);
  }
}

/*********************************************************/
/* hftest3:                                              */
/* Using insert_struc_recs, we will insert structured    */
/* records to a file, and then scan the file based on    */
/* it float attribute values of the records.             */
/* All the records with a value greater or equal to 50.0 */
/* will be retrived.                                     */
/*********************************************************/

void hftest3() 
{
  int fd, sd;  
  RECID recid, saved_recid;
  struct rec_struct record;
  float value;

  /* making sure file doesn't exits */
  unlink(FILE2);
  
  /* inserting records in file */
  insert_struc_recs(FILE2); 
  read_struc_recs(FILE2); 

  /* opening the file because we need 'fd' for opening the scan */
  if ((fd = HF_OpenFile(FILE2)) < 0)
  {
     HF_PrintError("Problem opening file.\n");
     exit(1);
  } 

  /* opening scan in the float field of struc rec_struc */
  /* with comparison operator greater or equal.           */
  /* So only those records with a value greater or equal  */
  /* to 'value' will be returned by the scan.             */

  value = 50.0;

  if ((sd = HF_OpenFileScan(fd,REAL_TYPE,sizeof(float),offsetof(struct rec_struct, 
float_val),GE_OP,(char *)&value)) <0)
  {
     HF_PrintError("Problem opening scan\n.");
     exit(1);
  }

  /* Getting the values from the scan */

  recid = HF_FindNextRec(sd,(char *)&record);

  printf("<< Scan records whose floating value >= 50 >>\n");
  while (HF_ValidRecId(fd,recid))
  {
     /* Save this record id for testing HF_GetThisRec() */
     if (record.int_val == RECORDVAL) saved_recid = recid;

     /* printing the record value */
     printf("scanned structured record: (%s, %f, %d)\n",
		record.string_val, record.float_val, record.int_val);

     recid = HF_FindNextRec(sd,(char *)&record);
  }

  /* Testing HF_GetThisRec() function */
  if (HF_GetThisRec(fd, saved_recid, (char *)&record) != HFE_OK) {
     HF_PrintError("Problem getting a record\n.");
     exit(1);
  }
  else {
     printf("<< fetch a record whose int value = %d >>\n",RECORDVAL);
     printf("record fetched by id: (%s, %f, %d)\n",
		record.string_val, record.float_val, record.int_val);
  }

  if (HF_CloseFileScan(sd) != HFE_OK) {
     HF_PrintError("Problem closing scan.\n");
     exit(1);
  }

  if (HF_CloseFile(fd) != HFE_OK) {
     HF_PrintError("Problem closing file.\n");
     exit(1);
  }

  if (HF_DestroyFile(FILE2) != HFE_OK) {
     HF_PrintError("Problem destroying the file.\n");
     exit(1);
  }

}

/*
 * Create 3 files, 
 */
void testPerso(){
   /* making sure FILE1 doesn't exist */
  unlink(FILE1);
  unlink(FILE2);

  /* Creating the HF file */
  printf("\nHF_CreateFile : %d\n", HF_CreateFile(FILE1,50));
  printf("HF_CreateFile : %d\n", HF_CreateFile(FILE2,80));

  /* Opening the files */ 
  printf("\nHF_OpenFile : %d\n", HF_OpenFile(FILE1));
  printf("HF_OpenFile : %d\n", HF_OpenFile(FILE2));


}

main()
{
  HF_Init();
 /*
  printf("*** begin of hftest1 ***\n");
  hftest1();
  printf("*** end of hftest1 ***\n");

  printf("*** begin of hftest2 ***\n");
  hftest2();
  printf("*** end of hftest2 ***\n");

  printf("*** begin of hftest3 *** \n");
  hftest3();
  printf("*** end of hftest3 *** \n");
  */
  printf("*** begin of test perso ***\n");
  testPerso();
  printf("*** end of test perso ***\n");
}

