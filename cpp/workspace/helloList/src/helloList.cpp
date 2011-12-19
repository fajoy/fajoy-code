#include <iostream> 
#include <stdio.h>
#include <stdlib.h>
#include <list> 

using namespace std;
int main(int argc, char *argv[], char *envp[]) {
	list<int> ints;
	list<int>::iterator it;

	ints.push_back(1);
	ints.push_back(2);
	ints.push_back(3);
	ints.push_back(4);
	it = ints.begin();
	printf("begin=%d\n", *it);
	// cout <<"begin="<<*it<<endl;
	it++;
	it++;
	printf("it+2=%d\n", *it);
	ints.insert(it, 0);
	cout << "size=" << ints.size() << endl;
	while (!ints.empty()) {
		cout << ints.front() << endl;
		ints.pop_front();
	}

	exit(EXIT_SUCCESS);

}
