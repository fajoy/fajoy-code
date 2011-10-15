#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef{
   char item[255];
}Chars;

Chars *split(char *str, const char *del) {
   char *s = strtok(str, del);
   char **arr;
   while(s != NULL) {
     *arr++ = s;
     s = strtok(NULL, del);
   }
   return arr;
 } 

int main(int argc,char *argv[],char *envp[])
{
   Chars str;
   str->item="test";
   printf("%d",strlen(str));

}
