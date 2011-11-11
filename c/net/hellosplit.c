#include <stdio.h>
#include <stdlib.h>
#include "Array.h"
#include "String.h"
#include "Array.c"

int main(int argc,char *argv[],char *envp[])
{
   String *str;
   char test[255]="hello|world|testo testltest|test |tes|1 ";
   str=newString(test,0);
   printf("%s  s=%d \n",str->item,strlen(str->item));
   Array *arr=split(str->item,"|");
   str->free(str);
   printf("%s  s=%d \n",str->item,strlen(str->item));

   int i;
   for(i=0;i<arr->length;i++){
     char *ss=ArrayGet(arr,i,char*);
     printf("arr[%d]=%s\n",i,ss);
   }

   return 0;
}
