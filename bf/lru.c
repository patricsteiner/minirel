#include <stdio.h>
#include <stdlib.h>
#include "minirel.h"
#include "pf.h"
#include "bf.h"
#include "lru.h"

LRU* lru_init(){ 
	LRU *new=malloc(sizeof(LRU));
	new->head=NULL;//at the beginning all free page are in the FREE list
	new->tail=NULL;
	new->number_of_page=0;

	return new; //the LRU structure is now initialized, this pointer will be used for every funtion using the LRU list
}


/**********************************************************************************************/
/* lru_add add the given page in parameters                                                   */
/**********************************************************************************************/

	
int lru_add(LRU* lru, BFpage *new_BFpage){
	//this BFpage become the most recently used, consequently it is the head of the LRU list
 	
	if(lru-> tail==NULL){ //first page in the LRU
		lru->head=new_BFpage;
		lru->tail=new_BFpage;
		new_BFpage->nextpage=NULL;
		new_BFpage->prevpage=NULL;	
	}
	
	lru->head->prevpage= new_BFpage;//just add the new BFpage before the head.

	new_BFpage->nextpage= lru->head;

	lru->head=new_BFpage; 	//The new page become consequently the head.
}




/**********************************************************************************************/
/* lru_find: return true if the page is in lru_list                                            */
/**********************************************************************************************/
 

char lru_find(LRU* lru, BFpage *page){ 

	if (lru->head == NULL) return BFE_EMPTY; //empty list case
	
	BFpage *pt= lru->head;
	do{
	     if(pt==page) return BFE_OK;// one address in memory for a page ==> the pointer are equal if they point the same page 
	     pt=pt->nextpage;
	}while(pt!=NULL); //stop the loop after the tail

	return 1;// not find
}



/**********************************************************************************************/
/* lru_remove: chooses a victim (last recently used page with pin 0) and removes it           */
/**********************************************************************************************/

int lru_remove(LRU* lru,BFpage** victim){
	if (lru->tail==NULL) return BFE_EMPTY; //empty list case

	//the tail is the last recently used page
	BFpage *pt= lru->tail;

	do{
	     if(pt->count==0){ // this is the victim
		pt->prevpage->nextpage=pt->nextpage; // changing pointer is enough to remove victim from the list
		pt->nextpage->prevpage=pt->prevpage; //now the victim is out of the list
		
		pt->nextpage=NULL;
		pt->prevpage=NULL;
		
		victim=&pt;

		return BFE_OK;// one address in memory for a page ==> the pointer are equal if they point the same page 
	 	}
		
	     pt=pt->prevpage;
	}while(pt!=NULL); //stop the loop after the tail
	
	return BFE_NOVICTIM; // if NULL is returned then the list is empty (first condition check, or after scanning all list: no page can be choose as a victim
}






