#ifndef __HF_H__
#define __HF_H__

#include <minirel.h>

#define HF_FTAB_SIZE    MAXOPENFILES    /* max number of HF files allowed */
#define MAXSCANS        MAXOPENFILES    /* max number of HF scans allowed */


/****************************************************************************
 * hf.h: external interface definition for the HF layer 
 ****************************************************************************/

/*
 * prototypes for HF-layer functions
 */
void 		HF_Init(void);
int 		HF_CreateFile(char *fileName, int RecSize);
int 		HF_DestroyFile(char *fileName);
int 		HF_OpenFile(char *fileName);
int		HF_CloseFile(int fileDesc);
RECID		HF_InsertRec(int fileDesc, char *record);
int	 	HF_DeleteRec(int fileDesc, RECID recId);
RECID 		HF_GetFirstRec(int fileDesc, char *record);
RECID		HF_GetNextRec(int fileDesc, RECID recId, char *record);
int	 	HF_GetThisRec(int fileDesc, RECID recId, char *record);
int 		HF_OpenFileScan(int fileDesc, char attrType, int attrLength, 
				int attrOffset, int op, char *value);
RECID		HF_FindNextRec(int scanDesc, char *record);
int		HF_CloseFileScan(int scanDesc);
void		HF_PrintError(char *errString);
bool_t          HF_ValidRecId(int fileDesc, RECID recid);
/*int             HF_HeaderInfo(int fileDesc, HFHeader *FileInfo);*/

/******************************************************************************/
/*	Error codes definition			  			      */
/******************************************************************************/
#define	HF_NERRORS		19	/* number of error codes used */

#define HFE_OK                   0  /* HF routine successful */
#define HFE_PF                  -1  /* error in PF layer */
#define HFE_FTABFULL            -2  /* # files open exceeds MAXOPENFILES  */
#define HFE_STABFULL            -3  /* # scans open exceeds MAXSCANS  */
#define HFE_FD                  -4  /* invalid file descriptor  */
#define HFE_SD                  -5  /* invalid scan descriptor */
#define HFE_INVALIDRECORD       -6  /* invalid record id  */
#define HFE_EOF                 -7  /* end of file  */
#define HFE_RECSIZE             -8  /* record size invalid */
#define	HFE_FILEOPEN		-9  /* File is open in file table */
#define	HFE_SCANOPEN		-10 /* Scan Open for the given file */
#define HFE_FILENOTOPEN		-11 /* File not open in file table */
#define HFE_SCANNOTOPEN		-12 /* Scan not open in scan table */
#define HFE_ATTRTYPE		-13 /* Invalid attribute type in file scan*/
#define HFE_ATTRLENGTH		-14 /* Invalid attribute length */
#define HFE_ATTROFFSET   	-15 /* Invalid attribute offset */
#define HFE_OPERATOR		-16 /* Invalid Operator in file scan */
#define HFE_FILE		-17 /* File not a MINIREL file */
#define HFE_INTERNAL            -18 /* Error which should not have */
			            /* occured (possibly corrupt file) */
#define	HFE_PAGE		-19 /* Unable to allocate page for file b'coz */
				    /* page number  > MAXPGNUMBER */

#define HFE_INVALIDSTATS        -20 /* meaningful only when STATS_XXX macros
                                       are in use */

/******************************************************************************/
/*	Data structure definition		  			      */
/******************************************************************************/
/* Data structure for HF header info */
/* Use this structure only as a reference. You'd better have your own
   design of HF header structure.  Bongki Moon, Feb/26/2017.
*/
#ifdef ONLY_FOR_REFERENCE
#define PAGENUM unsigned short int
#define RECNUM  unsigned short int

typedef struct {
    int RecSize;                 /* Record size */
    int RecPage;                 /* Number of records per page */
    int NumPg;                   /* Number of pages in file */
    int NumFrPgFile;             /* Number of free pages in the file */ 
} HFHeader;

#endif


/******************************************************************************/
/* The current HF layer error code or HFE_OK if function returned without one */
/******************************************************************************/
extern int	HFerrno;

#endif

