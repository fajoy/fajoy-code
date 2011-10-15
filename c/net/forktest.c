#include<stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct {
   char str[255];
   int length;
}mstring;

typedef struct {
   mstring *item;
   int length;
}mstrings;

char *newstring(int size){
   char *new=(char*)malloc(sizeof(char)*size);
   return new;
}
/*
mstring *split(char *str, const char *del) {
   char *s = strtok(str, del);
   char **arr;
   while(s != NULL) {
     *arr++ = s;
     s = strtok(NULL, del);
   }
   return arr;
 } 
*/

int main(int argc,char *argv[],char *envp[])
{
   mstring test;
   printf("%d",strlen(test.str));
   char *p;
   char *spl="|";
   char *str=&test.str;
   printf("\n> ");
   scanf("%s",str);
   p = strtok (str, spl);

   while (p != NULL)
   {
   printf ("%s\n", p);
   printf("%d\n",strlen(p));
   p = strtok (NULL, spl);
   }


   printf ("%s\n",str);

   return 0;
}


