#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>
#include "minirel.h"
#include "pf.h"
#include "../pf/pfUtils.h"
#include "hf.h"
#include "../hf/hfUtils.h"
#include "am.h"
#include "../am/amUtils.h"

void AM_Init(void){

}

int AM_CreateIndex(char *fileName, int indexNo, char attrType, int attrLength, bool_t isUnique){
	return AME_OK;
}

int AM_DestroyIndex(char *fileName, int indexNo){
	return AME_OK;
}

int AM_OpenIndex(char *fileName, int indexNo){
	return AME_OK;
}

int AM_CloseIndex(int fileDesc){
	return AME_OK;
}

int AM_InsertEntry(int fileDesc, char *value, RECID recId){
	return AME_OK;
}

int AM_DeleteEntry(int fileDesc, char *value, RECID recId){
	return AME_OK;
}

int AM_OpenIndexScan(int fileDesc, int op, char *value){
	return AME_OK;
}

RECID AM_FindNextEntry(int scanDesc){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;

	return res;
}

int AM_CloseIndexScan(int scanDesc){
	return AME_OK;
}

void AM_PrintError(char *errString){

}