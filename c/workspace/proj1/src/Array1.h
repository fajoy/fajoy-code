#include "Array.h"
#include <stdlib.h>
Array *split(char *str, const char *del) {
   char *root=strdup(str);
   char *sub=strtok(root, del);
   Array *list=newArray(100);
   int i;
   while(sub != NULL) {
     i=list->length;
     ArrayPush(list,sub);
     sub = strtok(NULL, del);
     char*ss=ArrayGet(list,i,char*);
     //printf("list[%d]=%s\n",i,ss);
   }
   return list;
} 
