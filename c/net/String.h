#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef String_h
#define String_h
typedef struct String{
   struct String *this;
   void (*free)(struct String*);
   char *item;
   int size;
}String;

void StringFree(struct String *this)
{
   free(this->item);
   free(this);
}
String *newString(char *str,int size){
   int str_size=strlen(str);
   if(size <=str_size)
         size=str_size;
   String *this=(String*)malloc(sizeof(String));
   this->this=this;
   this->free=StringFree;
   this->size=size;
   this->item=(char*)malloc(sizeof(char)*size);
   strcpy(this->item,str);
   return this;
}
#endif

