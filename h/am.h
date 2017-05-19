#ifndef __AM_H__
#define __AM_H__

#include <minirel.h>

#define AM_ITAB_SIZE    MAXOPENFILES    /* max number of AM files allowed */
#define MAXISCANS       MAXOPENFILES    /* max number of AM scans allowed */


/****************************************************************************
 * am.h: External interface to the AM layer
 ****************************************************************************/

/*
 * prototypes for AM functions
 */
void AM_Init		(void);
int  AM_CreateIndex	(char *fileName, int indexNo, char attrType,
			int attrLength, bool_t isUnique);
int  AM_DestroyIndex	(char *fileName, int indexNo); 
int  AM_OpenIndex       (char *fileName, int indexNo);
int  AM_CloseIndex      (int fileDesc);
int  AM_InsertEntry	(int fileDesc, char *value, RECID recId);
int  AM_DeleteEntry     (int fileDesc, char *value, RECID recId);
int  AM_OpenIndexScan	(int fileDesc, int op, char *value);
RECID AM_FindNextEntry	(int scanDesc);
int  AM_CloseIndexScan	(int scanDesc);
void AM_PrintError	(char *errString);

/*
 * AM layer constants 
 */
#define AM_NERRORS      24      /* maximun number of AM  errors */    

/*
 * AM layer error codes
 */
#define		AME_OK			0
#define         AME_PF                  (-1)
#define		AME_EOF			(-2)
#define 	AME_FULLTABLE		(-3)
#define 	AME_INVALIDATTRTYPE	(-4)
#define 	AME_FD  		(-5)
#define 	AME_INVALIDOP		(-6)
#define 	AME_INVALIDPARA		(-7)		/* Ok not to use */
#define 	AME_DUPLICATEOPEN	(-8)		/* Ok not to use */
#define 	AME_SCANTABLEFULL	(-9)
#define 	AME_INVALIDSCANDESC	(-10)
#define 	AME_UNABLETOSCAN	(-11)		/* Ok not to use */
#define 	AME_RECNOTFOUND		(-12)
#define 	AME_DUPLICATERECID	(-13)
#define 	AME_SCANOPEN		(-14)
#define 	AME_INVALIDATTRLENGTH	(-15)

/* 
 * define your own error codes here 
 */
#define		AME_UNIX		(-16)		/* Ok not to use */
#define         AME_ROOTNULL            (-17)
#define         AME_TREETOODEEP         (-18)
#define         AME_INVALIDATTR         (-19)
#define         AME_NOMEM               (-20)
#define         AME_INVALIDRECORD       (-21)
#define         AME_TOOMANYRECSPERKEY   (-22)
#define         AME_KEYNOTFOUND         (-23)
#define         AME_DUPLICATEKEY        (-24)

/*
 * global error value
 */
extern int AMerrno;

#endif
