#ifndef __BF_HEADER_H__
#define __BF_HEADER_H__

#include "minirel.h"
#include "bf.h"

/******************************************************************************/
/*   Type definition for pages of the BF layer.                               */
/******************************************************************************/

typedef struct BFpage {
  PFpage         fpage;       /* page data from the file                 */
  struct BFpage  *nextpage;   /* next in the linked list of buffer pages */
  struct BFpage  *prevpage;   /* prev in the linked list of buffer pages */
  bool_t         dirty;       /* TRUE if page is dirty                   */
  short          count;       /* pin count associated with the page      */
  int            pageNum;     /* page number of this page                */
  int            fd;          /* PF file descriptor of this page         */
} BFpage;



/**********************************************************************************************/
/*More errors use for BF layer                                                                */
/**********************************************************************************************/

#define BFE_UNPINNEDPAGE        (-54)
#define BFE_NOVICTIM            (-55)
#define BFE_EMPTY               (-56)


#endif
