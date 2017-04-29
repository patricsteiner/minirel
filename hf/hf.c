#include <stdio.h>
#include "pf.h"
#include "minirel.h"

void HF_Init(void){

}

int HF_CreateFile(char *fileName, int RecSize){
	return 0;
}

int HF_DestroyFile(char *fileName){
	return 0;
}

int HF_OpenFile(char *fileName){
	return 0;
}

int	HF_CloseFile(int fileDesc){
	return 0;
}

RECID HF_InsertRec(int fileDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_DeleteRec(int fileDesc, RECID recId){
	return 0;
}

RECID HF_GetFirstRec(int fileDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

RECID HF_GetNextRec(int fileDesc, RECID recId, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_GetThisRec(int fileDesc, RECID recId, char *record){
	return 0;
}

int HF_OpenFileScan(int fileDesc, char attrType, int attrLength, int attrOffset, int op, char *value){
	return 0;
}

RECID HF_FindNextRec(int scanDesc, char *record){
	RECID res;
	res.recnum = 0;
	res.pagenum = 0;
	return res;
}

int	HF_CloseFileScan(int scanDesc){
	return 0;
}

void HF_PrintError(char *errString){
	printf("\nHF_PrintError : %s\n", errString);
}

bool_t HF_ValidRecId(int fileDesc, RECID recid){
	return TRUE;
}
