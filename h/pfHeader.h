#ifndef __BF_HEADER_H__
#define __BF_HEADER_H_



#include "minirel.h"






/******************************************************************************/
/*   Type definition for file table and file header of the PF layer.          */
/******************************************************************************/
typedef struct PFhdr_str{
	int numpages; /* number of pages in the file */
}PFhdr_str;

typedef struct PFftab_ele{
	bool_t valid; /*set to TRUE when a file is open*/
	ino_t inode; /* inode number of the file */
	char *fname;/*file name */
	int unixfd;/*Unix file descriptor */
	PFhdr_str hdr;/*file header	*/
	short hdrchanged; /*true if file header has changed */
}PFftab_ele;


/**********************************************************************************************/
/*	More errors used for PF layer                                                                */
/**********************************************************************************************/

#define PFE_FILENOTEXISTS	-50
#define PFE_FILENOTINTAB	-51



#endif
