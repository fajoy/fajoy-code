#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
using namespace std; 

class Fibonacci{
public :
   static int get(int v){
      if(v>1){
         return Fibonacci::get(v-1)+ Fibonacci::get(v-2);
      }else{
         return 1;
      }
   }
};


int main(int argc, char *argv[], char *envp[]) {
   cout << Fibonacci::get(5)<<endl;
   exit(EXIT_SUCCESS);
}

