//============================================================================
// Name        : helloTime.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <iostream>
using namespace std;


int main() {

 	 time_t t;
 	 timeval tv;
 	 clock_t c;
	 t = time(NULL);
	 while(1){
		 if(t!=time(NULL)){
			 t=time(NULL);
			 gettimeofday(&tv,NULL);
			 c=clock();
			 tv.tv_usec+=0;
			 cout <<"t="<<t<<" tv_usec="<<tv.tv_sec<<" tv_usec="<<tv.tv_usec<<" c="<<c<<endl;
			 usleep(100000);
		 }
	 }

	return 0;
}
