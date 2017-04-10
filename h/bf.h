#ifndef __BF_H__
#define __BF_H__

/****************************************************************************
 * bf.h: external interface definition for the BF layer
 ****************************************************************************/

/*
 * size of buffer pool
 */
#define BF_MAX_BUFS     40

/*
 * size of BF hash table
 */
#define BF_HASH_TBL_SIZE 20

/*
 * prototypes for BF-layer functions
 */
void BF_Init(void);
int BF_AllocBuf(BFreq bq, PFpage **fpage);
int BF_GetBuf(BFreq bq, PFpage **fpage);
int BF_UnpinBuf(BFreq bq);
int BF_TouchBuf(BFreq bq);
int BF_FlushBuf(int fd);
void BF_ShowBuf(void);

/*
 * BF-layer error codes
 */
#define BFE_OK			0
#define BFE_NOMEM		(-1)
#define BFE_NOBUF		(-2)
#define BFE_PAGEFIXED		(-3)
#define BFE_PAGEUNFIXED		(-4)

#define BFE_PAGEINBUF		(-50)
#define BFE_PAGENOTINBUF	(-51)
#define BFE_HASHNOTFOUND	(-52)
#define BFE_HASHPAGEEXIST	(-53)

#define BFE_MISSDIRTY		(-60)
#define BFE_INVALIDTID		(-61)

/*
 * error in UNIX system call or library routine
 */
#define BFE_INCOMPLETEREAD	(-97)
#define BFE_INCOMPLETEWRITE	(-98)
#define BFE_MSGERR              (-99)
#define BFE_UNIX		(-100)

#endif

