#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

int main() {
	string host="ubu.fajoy.co.cc";
	int port=8001;

	sockaddr_in dest;
	int con_fd= socket(AF_INET, SOCK_STREAM, 0);

	bzero(&dest,sizeof(dest));
	dest.sin_family = AF_INET;

	char hostname[255];
	strcpy(hostname,host.c_str());

	struct hostent *server  = (struct hostent *) gethostbyname(host.c_str());
	dest.sin_addr = *((struct in_addr *) server->h_addr);

	//dest.sin_addr.s_addr =inet_addr(host.c_str());

	dest.sin_port = htons(port);

	int re=connect(con_fd, (struct sockaddr *) &dest,sizeof(dest));
	cout << re << endl;

	char buffer[255];
	bzero(buffer,255);
	recv(con_fd,buffer,sizeof(255),0);
	cout <<buffer<<endl;
	int flag;
	flag = fcntl(con_fd, F_GETFL, 0);
	fcntl(con_fd, F_SETFL, flag | O_NONBLOCK);
	cout <<"fd="<<con_fd<<endl;
	fflush(stdout);
	close(con_fd);
	//ofstream ss(con_fd);

	return 0;
}
