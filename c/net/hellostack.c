#include <stdio.h>
#include <stdlib.h>
#include "Array.h"
int main(int argc,char *argv[],char *envp[])
{
   int a=0;
   int b=1;
   printf("a=%d\nb=%d\n",a,b);
   printf("&a=%p &b=%p\n",&a,&b);
   push(stack,&a);
   push(stack,&b);
   int *arr;
   int arr_size=ToArray(stack,&arr);
   printf("arrsize=%d\n",arr_size);
   a=pop(stack,int);
   b=pop(stack,int);
   printf("a=%d b=%d\n",a,b);

   int i;
   for(i=0;i<arr_size;i++){
      int v=GetItem(arr,i,int);
      printf("arr[%d]=%d \n",i,v);
   }

   return 0;
}
