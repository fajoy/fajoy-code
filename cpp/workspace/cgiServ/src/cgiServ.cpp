#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <map>
#include "Request.h"
#include<libgen.h>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

Request *req;
struct sockaddr_in serv; /* socket info about our server */
struct sockaddr_in dest; /* socket info about the machine connecting to us */
int listen_fd; /* socket used to listen for incoming connections */
int con_fd;
int stdo_fd;
int PORTNUM = 8000;
void ssend(int fd, const char *msg) {
	send(fd, msg, strlen(msg), MSG_DONTWAIT);
}

int file_exists(const char * fileName) {
	struct stat buf;
	int i = stat(fileName, &buf);
	/* File found */
	if (i == 0) {
		return 1;
	}
	return 0;
}

void closeFD() {
	int s = FD_SETSIZE;
	for (s; s > 2; s--) {
		close(s);
	}
}

void resp_error() {
	ssend(con_fd, "HTTP/1.1 404 Not Found\n");
	ssend(con_fd, "Content-type: text/html\n\n");
	string html = "";
	html += "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">";
	html += "<html><head>";
	html += "<title>404 Not Found</title>";
	html += "</head><body>";
	html += "<h1>Not Found</h1>";
	html += "<p>The requested URL " + req->Page()
			+ " was not found on this server.</p>";
	html += "<hr>";
	html += "</body></html>";
	cout << html;
	fflush(stdout);
}
void fork_cgi() {

}
void exec_cgi() {
	//setenv("PATH", client->PATH, 1);
	string filePath = getenv("PWD");
	filePath += req->Page();
	/*
	 FILE *file=fopen(filePath,"r");
	 char tmp[80];
	 fgets(tmp, 80, file);
	 cloes(file);
	 string str=tmp;*/
	dprintf(stdo_fd, "exec=%s\n", filePath.c_str());
	fflush(stdout);
	int pid = 0;
	pid = fork();
	if (pid != 0) {
		int status;
		waitpid(pid, &status, WUNTRACED | WCONTINUED);
	} else {
		signal(SIGKILL, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		req->setvar();
		char _path[255];
		strcpy(_path,filePath.c_str());
		char *dir_name=dirname(&_path[0]);
		chdir(dir_name);
		execl("/bin/sh", "sh", "-c", filePath.c_str(), NULL);
	}
	//exec
	//execlp(filePath.c_str(), NULL);
	//resp_error();
}
void resp_head() {
	//shutdown()
	//shutdown(consocket,SHUT_RD);
	//shutdown(consocket,SHUT_RD);
	//close(1);
	//dup2(consocket,1);
	//int flag = fcntl(consocket, F_GETFL, 0);
	//fcntl(consocket, F_SETFL, flag | O_NONBLOCK);
	cout << "HTTP/1.1 200 OK\n";
	//ssend(consocket,"Accept-Asynchronous: response");
	cout << "Cache-Control: max-age=0\n";
	//dprintf(consocket,"Content-Length: 10\n");
	//ssend(consocket,"Accept-Ranges: bytes\n");
}
void resp_page() {

	cout << "Content-type: text/html\n\n";
	string filePath = getenv("PWD");
	filePath += req->Page();
	FILE *file = fopen(filePath.c_str(), "r");
	char tmp[80];
	while ((fgets(tmp, 80, file)) != NULL) {
		cout << tmp;
		fflush(stdout);
	}

	/*
	 ssend(con_fd, "<html><body>ok<br /></body></html>\n");
	 cout << req->REQUEST_URI() << "<br />";
	 cout << req->Page() << "<br />";
	 fflush(stdout);
	 */
	//printf("%s",buffer);
	//fflush(stdout);
	/*
	 for (int i = 0; i < 1000; i++) {
	 char tmp[20];
	 memset(tmp, 0, sizeof(tmp));
	 if (i % 10 == 0)
	 sprintf(tmp, "%d<br/>", i);
	 else
	 sprintf(tmp, "%d", i);
	 send(con_fd, tmp, strlen(tmp), MSG_DONTWAIT);
	 fflush(stdout);
	 usleep(10000);
	 }*/

}
void get_req() {

	char buffer[4096];
	int len = 0;
	len = recv(con_fd, buffer, 4096, 0);
	req = new Request();
	string head=buffer;
	string tmp;
	int index;
	int index1;
	int index2;

	//sscanf(buffer, "GET %s", str);
	//memset(str, 0, sizeof(str));
	//sscanf(buffer, "Host:%s", str);
	index=head.find_first_of("GET");
	if(index>=0){
		req->REQUEST_METHOD()="GET";
	}else{
		index=head.find_first_of("POST");
		if(index>=0)
			req->REQUEST_METHOD()="POST";
	}
	//dprintf(stdo_fd,"motd=%s\n",req->REQUEST_METHOD().c_str());
	index=head.find("/");
	index2=head.find(" ",index);
	tmp=head.substr(index,index2-index);
	//dprintf(stdo_fd,"i1=%d,i2=%d\n",index,index2);
	//remove(tmp.begin(),tmp.end(),' ');
	req->REQUEST_URI() = tmp;

	index=head.find("HTTP",index2);
	index2=head.find("\n",index);
	tmp=head.substr(index,index2-index);
	req->SERVER_PROTOCOL()=tmp;
	index=index2;

	string key;
	key="Host:";
	index1=head.find(key,index2);
	index1+=key.length();
	index2=head.find(":",index1);
	if(index2>index1){
	tmp=head.substr(index1,index2-index1);
	//int l=tmp.find(":",index1);
	req->HTTP_HOST()=tmp;
	}

	//dprintf(stdo_fd,"tmp=%s\n",tmp.c_str());

	//req->HTTP_HOST()+=str;

	sockaddr_in s_addr;
	socklen_t s_len=sizeof(s_addr);
	getsockname(con_fd, (struct sockaddr *)&s_addr, &s_len);
	req->SERVER_ADDR() = inet_ntoa(s_addr.sin_addr);

	stringstream ser_port(req->SERVER_PORT());
	ser_port << ntohs(s_addr.sin_port);
	req->SERVER_PORT() = ser_port.str();

	req->REMOTE_ADDR() = inet_ntoa(dest.sin_addr);

	stringstream rem_port(req->REMOTE_PORT());
	rem_port << ntohs(dest.sin_port);
	req->REMOTE_PORT() = rem_port.str();



}

/*
 void set_sock(int sock_fd){
 linger linger;
 linger.l_onoff = 1;
 linger.l_linger = 0;
 setsockopt(sock_fd, SOL_SOCKET, SO_LINGER, &linger, sizeof(linger));
 }*/

void reps_over(int signo) {
	fflush(stdout);
	fflush(stdout);
	shutdown(con_fd, SHUT_RDWR);
	close(con_fd);
	closeFD();
	close(2);
	close(1);

}

void fork_req() {
	int cid = 0;
	cid = fork();
	if (cid != 0) {
		return;
	}
	signal(SIGKILL, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGKILL, reps_over);
	signal(SIGTERM, reps_over);
	signal(SIGQUIT, reps_over);

	close(listen_fd);
	dup2(con_fd, 0);
	dup2(con_fd, 1);
	dup2(con_fd, 2);

	get_req();
	close(0);
	shutdown(con_fd, SHUT_RD);
	string filePath = getenv("PWD");
	filePath += req->Page();
	const char* file_name = filePath.c_str();
	if (file_exists(file_name)) {

		if (opendir(file_name) != NULL) {
			resp_error();
		} else {

			resp_head();
			fflush(stdout);
			if (req->isCgi()) {
				//cout<<"is cgi<br />";
				//dprintf(stdo_fd, "is cgi\n");
				//fflush(stdout);
				exec_cgi();
			} else {
				//cout<<"not is cgi<br />";
				//dprintf(stdo_fd, "not is cgi\n");
				//fflush(stdout);
				resp_page();
			}
			fflush(stdout);

		}

	} else {
		resp_error();
	}

	reps_over(SIGQUIT);
	exit(EXIT_SUCCESS);
}

static void handlerChld(int signo) {
	if (signo == SIGCHLD) {
		int status;
		//int chld_pid =
		waitpid(-1, &status, WUNTRACED | WCONTINUED);
	}
}
void main_kill(int signo) {
	shutdown(listen_fd, SHUT_RDWR);
	close(listen_fd);
}

void serv_bind() {

	memset(&serv, 0, sizeof(serv)); /* zero the struct before filling the fields */
	serv.sin_family = AF_INET; /* set the type of connection to TCP/IP */
	serv.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
	serv.sin_port = htons(PORTNUM); /* set the server port number */
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	/* bind serv information to mysocket */
	bind(listen_fd, (struct sockaddr *) &serv, sizeof(struct sockaddr));
}
void get_linsten_port() {
	printf("Input Server Port:");
	char input[10];
	fgets(input, 9, stdin);
	if (atoi(input) != 0)
		PORTNUM = atoi(input);
}
void chPWD() {
	string PWD = getenv("PWD");
	PWD += "/ras";
	chdir(PWD.c_str());
	setenv("PWD", PWD.c_str(), 1);
}

int main() {
	signal(SIGCHLD, handlerChld);
	signal(SIGKILL, main_kill);
	chPWD();
	cout << getenv("PWD") << endl;
	stdo_fd = dup(1);
	get_linsten_port();
	serv_bind();
	//int flag = fcntl(mysocket, F_GETFL, 0);
	//fcntl(mysocket, F_SETFL, flag | O_NONBLOCK);
	/* start listening, allowing a queue of up to 1 pending connection */
	listen(listen_fd, 1);
	while (1) {
		socklen_t socksize = sizeof(struct sockaddr_in);
		con_fd = accept(listen_fd, (struct sockaddr *) &dest, &socksize);
		fork_req();
	}
	return 0;
}
