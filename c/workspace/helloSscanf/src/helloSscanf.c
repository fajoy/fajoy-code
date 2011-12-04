/*
 ============================================================================
 Name        : helloSscanf.c
 Author      : 
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>

int main(void) {
	char str[255]="abcdef\n11zzzz\n";
	char parse[255]="\0";
	char empty[2]="\0\n";

	memset(parse,0,sizeof(parse));
	int has_abc=sscanf(str,"abc%s",parse);
	printf("has_abc=%d parse=%s\n",has_abc,parse);

	memset(parse,0,sizeof(parse));
	int has_abce=sscanf(str,"abce%s",parse);
	printf("nohas_abce=%d parse=%s\n",has_abce,parse);

	memset(parse,0,sizeof(parse));
	int no_char=sscanf(empty,"%[^n]",parse);
	printf("no_char=%d parse=%s\n",no_char,parse);

	memset(parse,0,sizeof(parse));
	char lines[]="1y2y\n3yz\0";
	char *linep=lines;
	printf("linep=%p\n",linep);
	int has_line=sscanf(linep,"%*[^z]%[^\n]",parse);
	printf("linep=%p\n",linep);
	printf("has_line=%d parse=%s\n",has_line,parse);

	return EXIT_SUCCESS;
}

