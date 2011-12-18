#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <map> 

using namespace std; 
int main(int argc, char *argv[], char *envp[]) {
 map<char,int> ints;
 ints['a']=1;
 ints['b']=2;
 ints['c']=3;
 ints['d']=4;
 map<char,int>::iterator it;

 it=ints.begin();
 printf("begin%c=%d\n",(*it).first,(*it).second);
// cout <<"begin="<<*it<<endl;
 it++;
 it++;
 printf("it+2 %c=%d\n",(*it).first,(*it).second);
 ints.insert(it,pair<char,int>('e',0));
 printf("%c=%d\n",'d',ints['d']);
 cout << "count d="<<ints.count('d')<<endl;
 cout << "count f="<<ints.count('f')<<endl;
 printf("%c=%d\n",'f',ints['f']);
 cout << "count f="<<ints.count('f')<<endl;
 printf("size=%d\n",ints.size());
 for(it=ints.begin();it!=ints.end();it++){
 printf("%c=%d\n",(*it).first,(*it).second);
 }
 cout<<endl;
 ints.erase('e');
 ints['b']=10;
 cout << "count b="<<ints.count('b')<<endl;
 printf("size=%d\n",ints.size());

 cout<<endl;
 while(!ints.empty()){
    cout<< (*ints.begin()).first<<"="<<(*ints.begin()).second<<endl;
    ints.erase(ints.begin());
 }


 exit(EXIT_SUCCESS);

}
