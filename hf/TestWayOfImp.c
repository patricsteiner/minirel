#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define PF_PAGE_SIZE 4092
typedef struct fcouple{
	int pagenum; 
	 float key;
	
}fcouple; 

typedef struct fnode{
	int is_leaf;            /* first element checked, is the boolean a node ? */ 
	int num_keys;		/* number of key into the node*/
	fcouple couple[50];
	fcouple* couple2;		/* all the couple key + pointer ( the page number of the child) , has to be malloc */
	int last_pt;            /* there is one more pointer than keys */
	
	
} fnode;

char pfpagebuf[4092];

void sum( char* valuei, char* valuef, char* valuec){
	int a=350;
	float f=1.0;
	float f2;
	char string[100];
	sprintf((char*) string, "entry%d",102);
	memcpy((float*) &f2, valuef, sizeof(float));
	printf( "valuef %f , memcpy %f\n", *((float*)valuef),f2);
	printf("string %s , %d\n ", string, sizeof(string));
	printf(" size float %d \n", sizeof(float));
	printf( "res de comparaison %d \n ", strncmp((char*) &a, valuei, 4));
	printf( "res de comparaison flooooooooat %d \n ", f>=*((float*)valuef) );
	printf( " res de comparaions string %d \n", strncmp((char*) string, valuec, sizeof(string)));
}
int main (){
	int y=0;/*  first record is written on the pfpagebuf */
	int N=1; /* only one record */
        int rec_size=28;
        int rec_page=0; 
	int i=0;
	/* test for scan */
	int value; 
	float valuef;
	char string[10];
	int rest,n_packed,x,bitmap_size;
	/* test for memcpy with struct*/
	int it[50],j;
	fnode fnod;
	fnode fnoda;
	fcouple* vali;

	fcouple* coupl;
	char val[400];

	j=0;
	for(j=0;j<50;j++){
		 fnod.couple[j].key=2.0;
		 fnod.couple[j].pagenum=j;
	}
	
	fnod.last_pt=10;
	fnod.num_keys=10;
	memcpy((char*) val, (fnode*) &fnod, sizeof(fnode));
	fnoda.couple2=(fcouple*) (val+2*sizeof(int));

	printf("last value cast pour struct %f %d %d\n", *((float*)((val+3*sizeof(int)) )), *((int*) (val+2*sizeof(int)*50+2*sizeof(int)+8)), fnoda.couple2[49].pagenum);
	/*memcpy((char*) val, (couple*) coupl,50*sizeof(couple));
	vali=(couple*) val;  cast 
	printf("last value cast pour struct %f %d \n", vali[49].key,vali[49].pagenum);*****/



	memcpy((int*) it,(char*) val,  50*sizeof(int));
	printf("last value %d \n", it[49]);

	
	sprintf((char*) string, "entry%d", 100);
	valuef=50.0;
	value=300;
       
   
	
	x=0;
	rest= ( (PF_PAGE_SIZE  - sizeof(int) ) % (rec_size) )*8 ;  /* it is the number of unused bits in a packed formated page*/
	n_packed= (PF_PAGE_SIZE  - sizeof(int)) / (rec_size); /*since it is int operands, it will be the floor of quotient == the number of rec in packed formated page */
	
	while ( (rest + x*(8*rec_size))<n_packed) {x++;} /* n_packed is the maximum size of the bitmap (== number of records) , if x is not 0 then the size of bitmap will be (n_packed-x)*/
	rec_page= n_packed-x;
	/*bitmapsize*/
	 if( ((PF_PAGE_SIZE*8) - (8*((rec_page*rec_size)+sizeof(int)) + rec_page))< rec_page%8) rec_page--; 
	bitmap_size = (rec_page%8)==0 ?(rec_page / 8): (rec_page/8)+1;

	printf("there is %d records in a page %d , %d\n", rec_page, n_packed/8,bitmap_size);


	/********************************** write in pfpagebuf ******************/
	for (i=0;i<=bitmap_size;i++) pfpagebuf[i]=0;
        pfpagebuf[((x-1)/8)] &= (2^((x-1)%8) )- 1;
       
	memcpy((char*) (pfpagebuf + (rec_size/8)+sizeof(int)+ y*rec_size) , (char*) "0123456789012345678901234567ffffff", rec_size);
	memcpy((char*) pfpagebuf + (rec_size/8), (int*) &N , sizeof(int));

	printf("page %s \n", pfpagebuf+(rec_size/8)+sizeof(int));

	memcpy((int *)&N, pfpagebuf+(rec_size/8), sizeof(int)); 
	printf("N before increment %d \n", N);
	N++;
        memcpy((char*) pfpagebuf + (rec_size/8), (int*) &(N) , sizeof(int));
	memcpy((int *)&N, pfpagebuf+(rec_size/8), sizeof(int)); 
	printf("N after increment %d \n", N);
        
	sum( (char*) &value, (char*)&valuef, string);
}



	
	
	
