//============================================================================
// Name        : helloTime.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <time.h>
#include <iostream>
using namespace std;

int main() {


 	 //timeval tv;


	 while(1){
		//gettimeofday(&tv,NULL);
		printf("c=%d %d\n",clock(),CLOCKS_PER_SEC);
		//cout <<" tv_usec="<<tv.tv_sec<<" tv_usec="<<tv.tv_usec<<" c="<<c<<endl;
		fflush(stdout);
		clock_t c;

		sleep(1);
	 }

	return 0;
}
