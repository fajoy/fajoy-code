#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <ctype.h>
using namespace std;

int main() {
	char str[] = "test@test.sss\nma@gs.ss";
	int nmatch = 1;
	int regex_flag = REG_EXTENDED | REG_ICASE;
	regex_t preg;
	char * pattern = "[A-Z0-9\\._%+-]+@[A-Z0-9\\.-]+\\.[A-Z]{2,4}"; // 簡單的 Email 格式
	regmatch_t pmatch[nmatch];
	if (regcomp(&preg, pattern, regex_flag) != 0) {
		printf("regexp comp error.\n");
		exit(0);
	}
	int data_length;
	int i;
	char msg[255];
	if (regexec(&preg, str, nmatch, pmatch, 0) == 0) // 非 0 為 no match
	{
		for (i = 0; i < nmatch && pmatch[i].rm_so >= 0; ++i) {
			memset(msg, 0, sizeof(msg));
			data_length = pmatch[i].rm_eo - pmatch[i].rm_so; // 透過 *data_length 得知資料該取的長度
			strncpy(msg, str + pmatch[i].rm_so, data_length);
			printf("%s\n", msg);
		}
	}
	regfree(&preg);
	printf("over\n");
	exit(EXIT_SUCCESS);
}
