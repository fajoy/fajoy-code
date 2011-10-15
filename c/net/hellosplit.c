#include <stdio.h>
#include <stdlib.h>
#include "Array.h"
#include "String.h"
Array *split(char *str, const char *del) {
   char *root=strdup(str);
   char *sub = strtok(root, del);
   Array *list=newArray(100);
   int i;
   while(sub != NULL) {
     i=list->length;
     ArrayPush(list,sub);
     sub = strtok(NULL, del);
     char*ss=ArrayGet(list,i,char*);
     printf("list[%d]=%s\n",i,ss);
   }
   return list;
} 

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
