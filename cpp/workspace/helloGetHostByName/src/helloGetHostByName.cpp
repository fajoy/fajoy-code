#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
using namespace std;

int main() {
	struct hostent *server = (struct hostent *) gethostbyname("ubu.fajoy.co.cc");
	struct in_addr addr;
	unsigned char ip_b[4];
	int ip=0;
	if(server){
		while(*server->h_aliases)
			printf("alias:%s\n",*server->h_aliases++);

		while(*server->h_addr_list){
			bcopy(*server->h_addr_list,&addr,sizeof(addr));
			bcopy(*server->h_addr_list,&ip_b[0],4);
			memcpy(&ip,server->h_addr_list,sizeof(ip));
			//memcpy(ip_b,&addr,4);
			bcopy(&addr,&ip_b[0],4);

			//unsigned char *p=(unsigned char*)&addr;
			memcpy(&ip,&addr,sizeof(ip));
			//ip_b[3]=*p++;
			//ip_b[2]=*p++;
			//ip_b[1]=*p++;
			//ip_b[0]=*p++;
			printf("addr :%s\n",inet_ntoa(addr));
			printf("addr :%x\n",ip);
			printf("addr :%d.%d.%d.%d\n",ip_b[0],ip_b[1],ip_b[2],ip_b[3]);
			server->h_addr_list++;
		}
	}

	return 0;
}
