#ifndef __MINIREL_H__
#define __MINIREL_H__

/****************************************************************************
 * minirel.h: global defintions
 ****************************************************************************/

/*
 * Types for Boolean and real
 */
typedef enum { FALSE = 0, TRUE } bool_t;
typedef float  real;

/*
 * supported data types
 */
#define NTYPES          3	/* number of supported types */
#define INT_TYPE        'i'
#define REAL_TYPE       'f'
#define STRING_TYPE     'c'

/*
 * supported operator types for comparison
 */
#define EQ_OP           1       /* attribute == value */
#define LT_OP           2       /* attribute < value */
#define GT_OP           3       /* attribute > value */
#define LE_OP           4       /* attribute <= value */
#define GE_OP           5       /* attribute >= value */
#define NE_OP           6       /* attribute # value */
#define ALL_OP          7       /* if null all records are to be returned */

/*
 * configuration of system resources
 */
#define MAXOPENFILES    20      /* maximum # of files open at one time  */

#ifndef PAGE_SIZE
#define PAGE_SIZE		4096
#define PF_PAGE_SIZE            (PAGE_SIZE-sizeof(int))
#endif


/******************************************************************************/
/*   Type definition for RECID, record identification in the HF layer.        */
/******************************************************************************/
typedef  struct _hf_record_identification {
	int	pagenum;
	int	recnum;
} RECID;

/******************************************************************************/
/*   Type definition for pages of the PF layer.                               */
/******************************************************************************/
typedef struct PFpage {
    char pagebuf[PAGE_SIZE];		/* actual page data             */
} PFpage;

/******************************************************************************/
/*   Type definition for BFreq, buffer control block for BF and PF layers.    */
/******************************************************************************/
typedef struct _buffer_request_control {
    int         fd;                     /* PF file descriptor */
    int         unixfd;                 /* Unix file descriptor */
    int         pagenum;                /* Page number in the file */
    bool_t      dirty;                  /* TRUE if page is dirty */
} BFreq;

#endif

