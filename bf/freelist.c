#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freeList.h"


int main(){
	Freelist* fl = fl_init(3);
	fl_print(fl);

	BFpage* bp1;
	bp1 = fl_give_one(fl);
	fl_print(fl);
	bp1->fd = 10;
	bp1->dirty = TRUE;
	bp1->pageNum = 7;
	bp1->count = 10;
	strcpy(bp1->fpage.pagebuf, "some data");

	fl_add(fl, bp1);
	fl_add(fl, bp1);
	
	fl_print(fl);

	fl_free(fl);
}

Freelist *fl_init(int max_size){
	int i;
	BFpage* newPage;
	BFpage* prev;

	Freelist* fl = malloc(sizeof(Freelist)); //address of the beginning of the free space 
	
	if (fl == NULL){exit(EXIT_FAILURE);}

	fl->max_size = max_size;
	fl->size = 1;
	fl->head = malloc(sizeof(BFpage)); //allocate space for the first BFpage
	if(fl->head == NULL){exit(EXIT_FAILURE);}

	strcpy(fl->head->fpage.pagebuf, "");
	prev = fl->head;
	prev->dirty = FALSE;
	prev->count = 0;
	prev->pageNum = 0;
	prev->fd = 0;

	for(i = 1; i< max_size; i++){
		newPage = malloc(sizeof(BFpage));
		if(newPage == NULL){exit(EXIT_FAILURE);}
		strcpy(newPage->fpage.pagebuf, "");
		prev->nextpage = newPage;
		prev = newPage;
		fl->size += 1;
	}

	newPage->nextpage = NULL;
	return fl;
}


BFpage* fl_give_one(Freelist* fl){
	if(fl == NULL){exit(EXIT_FAILURE);}
	if(fl->size == 0){return NULL;}
	if(fl->head != NULL){
		BFpage* res = fl->head;
		fl->head = fl->head->nextpage;
		fl->size -= 1;
		return res;
	}
}

int fl_add(Freelist* fl, BFpage* bpage){
	if(fl == NULL || bpage == NULL){exit(EXIT_FAILURE);}
	if(fl->size >= fl->max_size){printf("\nFL ERROR : Max Size reached");exit(EXIT_FAILURE);}

	//Cleaning the data into the BFPage
	bpage->dirty = FALSE;
	bpage->count = 0;
	bpage->pageNum = 0;
	bpage->fd = 0;
	bpage->prevpage = NULL;

	//Adding it at the beginning of the freelist
	bpage->nextpage = fl->head;
	fl->head = bpage;
	fl->size += 1;
}


int fl_free(Freelist* fl){
	int i;
	if(fl == NULL){exit(EXIT_FAILURE);}
	if(fl->head != NULL){
		BFpage* el = fl->head;
		BFpage* temp;	
		for(i = 0; i < fl->size; i++){
			temp = el;
			el = el->nextpage;
			free(temp);
		}
		free(fl);
		return 0;
	}
	return 1; //error
}

void fl_print(Freelist* fl){
	BFpage* ptr = fl->head;

	printf("\n------PRINTING LIST------");
	printf("\n This freelist has %d entries\n", fl->size);
	printf("\npagebuf data\tdirty\tcount\tpageNum\tfd ");
	while(ptr!=NULL){
		printf("\n[%s]", ptr->fpage.pagebuf);
		printf("\t\t[%d]", ptr->dirty);
		printf("\t[%d]", ptr->count);
		printf("\t[%d]", ptr->pageNum);
		printf("\t[%d]", ptr->fd);
		ptr = ptr->nextpage;
	}
	printf("\n------END PRINTING ------\n\n");
	return;
}

