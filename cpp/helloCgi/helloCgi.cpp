#include <iostream> 
#include <stdio.h>
#include <stdlib.h>

using namespace std;
 int main(int argc, char *argv[], char *envp[]) {
 cout <<"Content-type: text/html" <<endl<<endl;

 cout <<"hello"<<std::endl;
 int i=argc;
 for(i=0;i<argc;i++){
    cout <<"argv["<<i<<"]" <<argv[i]<<std::endl;
 }

 i=0;
 while(envp[i])
    std::cout <<"envp["<<i++<<"]" <<envp[i]<<std::endl;
   
 exit(EXIT_SUCCESS);

}
