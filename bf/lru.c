#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include "minirel.h"
#include "pf.h"
#include "bf.h"
#include "lru.h"

/**********************************************************************************************/
/* lru_init initializes and return an empty lru list                                               */
/**********************************************************************************************/

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
 	
	if(lru->tail==NULL){ //first page in the LRU
		printf("liste vide \n");
                new_BFpage->nextpage=NULL;
		new_BFpage->prevpage=NULL;
		lru->head=new_BFpage;
		lru->tail=new_BFpage;
			
	}
	else{
		lru->head->prevpage= new_BFpage;//just add the new BFpage before the head.

		new_BFpage->nextpage= lru->head;

		lru->head=new_BFpage; 	//The new page become consequently the head.
		
	}
	lru->number_of_page+=1;
	return BFE_OK;
}




/**********************************************************************************************/
/* lru_find: return true if the page is in lru_list                                            */
/**********************************************************************************************/
 

char lru_find(LRU* lru, BFpage *page){ 

	if (lru->head == NULL) return BFE_NOBUF; //empty list case
	
	BFpage *pt= lru->head;
	do{
	     if(pt==page) return BFE_OK;// one address in memory for a page ==> the pointer are equal if they point the same page 
	     pt=pt->nextpage;
	}while(pt!=NULL); //stop the loop after the tail

	return BFE_PAGENOTINBUF;// not find
}



/**********************************************************************************************/
/* lru_remove: chooses a victim (last recently used page with pin 0) and removes it           */
/**********************************************************************************************/

int lru_remove(LRU* lru,BFpage** victim){
	
	if (lru->tail==NULL) return BFE_PAGENOTINBUF; //empty list case

	//the tail is the last recently used page
	BFpage *pt= lru->tail;

	do{
	     printf("1\n");
	     if(pt->count==0){ // the victim is pointed by pt

		/* to remove from the lru list, change pointer is enough */ 
		if (pt != lru->head){
	

  
			if(pt!=lru->tail) {
				pt->prevpage->nextpage=pt->nextpage; 
				pt->nextpage->prevpage=pt->prevpage;
			 }
			
			else{	
				printf("hhhhhhhhhho\n");
				pt->prevpage->nextpage=NULL;/*tail victim ==> the page before tail becomes the tail	*/	lru->tail=pt->prevpage;
			}
		}
		else{
			if(pt!=lru->tail) {
					pt->nextpage->prevpage=NULL;// head victim ==> next page becomes head
					lru->head=pt->nextpage;
			 }
			else{// the head is the tail
				lru->head=NULL;
				lru->tail=NULL;
			}
					
		
		}
		/************ victim is removed ********************************/
		pt->nextpage=NULL;
		pt->prevpage=NULL;
		lru->number_of_page-=1;
		(*victim)=pt;
		return BFE_OK;
	    }
		
		
	     pt=pt->prevpage;
	}while(pt!=NULL); //stop the loop after the tail
	
	return BFE_PAGENOTINBUF; // if NULL is returned then the list is empty (first condition check, or after scanning all list: no page can be choose as a victim
}



/**********************************************************************************************/
/* lru_print: print status of LRU, number of pages, and id of pages                           */
/**********************************************************************************************/


void lru_print(LRU* lru){
	if(lru->head==NULL){
		printf(" \n EMPTY List \n\n");
		return;
	}
	printf(" There is %d pages in the LRU list \n\n ", lru->number_of_page);

	BFpage *pt= lru->head;

	do{
             printf(" Data: %s             Dirty flag: %d           pin: %d             Fd - page number: %d - %d \n\n", pt->fpage.pagebuf, pt->dirty, pt->count, pt->fd, pt->pageNum);

	     pt=pt->nextpage;
	}while(pt!=NULL); //stop the loop after the tail
	return;
	
}

/**********************************************************************************************/
/* lru_mtu put the given page in parameters, head of the list                                 */
/**********************************************************************************************/
int lru_mtu(LRU* lru, BFpage* mtu_page){ //this mtu_page (considered already in the list) is hit ==> most recently used page which become the head
	if (lru->tail==NULL) return BFE_NOBUF; //empty list case ==> not normal because the mtu_page is supposed to be in the LRU
	
	BFpage *pt= lru->head;
	do{
             if(pt==mtu_page) {
		if (pt != lru->head){ //mtu_page is not the head
			/* mtu_page has to be removed from its place in the list */
			if(pt!=lru->tail) {
				pt->prevpage->nextpage=pt->nextpage; 
				pt->nextpage->prevpage=pt->prevpage;
			 }
			
			else{
				pt->prevpage->nextpage=NULL;/*tail victim ==> the page before tail becomes the tail	*/	
				lru->tail=pt->prevpage;
			}
			/* now mtu_page is removed, and become the head of the list */
			mtu_page->nextpage=lru->head;
			lru->head->prevpage=mtu_page;
			lru->head=mtu_page;
			
		}
		else{
			// mtu_page is already the head.
		}
		return BFE_OK;
	     }	     
	     pt=pt->nextpage;
	}while(pt!=NULL); //stop the loop after the tail

	return BFE_PAGENOTINBUF;
}


int main(){
	LRU* lru;
	BFpage *victim=malloc(sizeof(BFpage*));
	BFpage *page1=malloc(sizeof(BFpage));
	BFpage *page2=malloc(sizeof(BFpage));
	strncpy(page1->fpage.pagebuf, "123456", 7);
	strncpy(page2->fpage.pagebuf, "héhéhohoho", 11);
	lru=lru_init();
	page1->dirty=FALSE;
	page2->dirty=FALSE;
	page1->fd=1;
	page2->fd=2;
	page2->pageNum=2;
	page1->pageNum=1;
	page1->count=0;
	page2->count=0;

	printf("first add: %d \n", lru_add(lru,page1));
	
	printf( "null ou pas %d \n " , page1->nextpage==NULL);
	printf("2 add: %d \n", lru_add(lru,page2));
	printf("find page1: %d \n", lru_find(lru,page1));
	
	

	
	//printf("%s /n",lru->tail->nextpage->pageNum);
	printf("first remove: %d \n",lru_remove(lru,&victim));
	printf("mtu héhéhhoho %d \n", lru_mtu(lru,page2));
	
	printf("remove number of the page: %d \n", (victim)->pageNum);
printf("2 remove: %d \n",lru_remove(lru,&victim));
	printf( "null ou pas %d \n " , page1->nextpage==NULL);
	lru_print(lru);
	printf("remove number of the page: %d \n", (victim)->pageNum);
	
	
}

