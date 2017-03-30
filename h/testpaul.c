#include <stdio.h>
#include "math.h"



void main(){
char *str[100];
int length = snprintf(NULL, 0,"%d",42);
/* int snprintf(char *str, size_t size, const char *format, ...); */

snprintf(str, "%d", 42);


int x=(int)((ceil(log10(20))+1)*sizeof(char));

printf("ddd ceil %d \n",x);

}
