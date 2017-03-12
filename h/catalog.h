#ifndef __MINIREL_H__
#include "minirel.h"
#endif

#ifndef __CATALOG_H__
#define __CATALOG_H__

/* catalog.h:  Everything you ever wanted to know about catalogs */ 

/* Catalogs: externally defined global variables.                */
extern int relcatFd, attrcatFd;

/* names of catalog relations */

#define RELCATNAME              "relcat"
#define ATTRCATNAME		"attrcat"


/* Maximum size of relation and attribute names */
#define MAXNAME                 12

/* structures of relcat relation */

typedef struct{
    char relname[MAXNAME + 1];	/* relation name			*/
    int  relwid;		/* tuple width (in bytes)		*/
    int  attrcnt;		/* number of attributes			*/
    int  indexcnt;		/* number of indexed attributes		*/
    char primattr[MAXNAME + 1];	/* name of primary attribute		*/
} RELDESCTYPE;

#define RELDESCSIZE            sizeof(RELDESCTYPE)
#define RELCAT_NATTRS		5
#define relCatOffset(field)    offsetof(RELDESCTYPE,field)


/* structure of attrcat relation */

typedef struct{
    char relname[MAXNAME + 1];	/* relation name			*/
    char attrname[MAXNAME + 1];	/* attribute name			*/
    int  offset;		/* attribute offset in tuple		*/
    int  attrlen;		/* attribute length			*/
    int  attrtype;		/* attribute type			*/
    bool_t  indexed;		/* if the field is indexed or not       */
    int  attrno;		/* attr number (used in index name)	*/
} ATTRDESCTYPE;

#define ATTRDESCSIZE           sizeof(ATTRDESCTYPE)
#define ATTRCAT_NATTRS		7
#define attrCatOffset(field)   offsetof(ATTRDESCTYPE,field)

/* Used for the retrieval of attributes from the attribute catalog */
/* ALL retrieves all the attributes from the attribute catalog    */
/* associated with a relation, and INDEXED only those associated  */
/* with an index                                                  */

#define ALL        0
#define INDEXED    1

/*
 * README README README README README 
 *
 * extra types not supported by the rest of minirel
 * Take a look at minirel.h in the public directory
 * for definition of NTYPES and the other three types
 * which you must already have in minirel.h
 */
#define BOOL_TYPE       NTYPES
#define TYPE_TYPE       (NTYPES + 1)

#endif