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
#include <sys/time.h>
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

fd_set rfds_src;
fd_set wfds_src;
fd_set rfds;
fd_set wfds;
typedef struct reqdata {
	char h1[255];
	char h2[255];
	char h3[255];
	char h4[255];
	char h5[255];
	char p1[5];
	char p2[5];
	char p3[5];
	char p4[5];
	char p5[5];
	char f1[255];
	char f2[255];
	char f3[255];
	char f4[255];
	char f5[255];
} reqdata;
char *query;
reqdata req;
void init_req() {
	query = getenv("QUERY_STRING");
	/*
	if (query == NULL) {
		char
				testquery[] =
						"h1=ubu.fajoy.co.cc&p1=8001&f1=t1.txt&h2=&p2=&f2=&h3=&p3=&f3=&h4=&p4=&f4=&h5=&p5=&f5=";
		query = testquery;
	}*/
	memset(&req, 0, sizeof(req));
	sscanf(query, "h1=%[^&]&p1=%[^&]&f1=%[^&]&"
		"h2=%[^&]&p2=%[^&]&f2=%[^&]&"
		"h3=%[^&]&p3=%[^&]&f3=%[^&]&"
		"h4=%[^&]&p4=%[^&]&f4=%[^&]&"
		"h5=%[^&]&p5=%[^&]&f5=%[^&]", &req.h1[0], &req.p1[0], &req.f1[0],
			&req.h2[0], &req.p2[0], &req.f2[0], &req.h3[0], &req.p3[0],
			&req.f3[0], &req.h4[0], &req.p4[0], &req.f4[0], &req.h5[0],
			&req.p5[0], &req.f5[0]);

}

class SerCon {
public:
	int index;
	char host[255];
	int port;
	char filename[255];
	int con_fd;
	bool is_con;
	bool is_openfile;
	string recv_buf;
	string send_buf;
	int mode;
	timeval send_t;
	timeval time_out;
	SerCon() {

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
	void initobg(int i) {

		index = i;
		mode = 1;
		is_openfile = false;
		is_con = false;
		memset(host, 0, sizeof(host));
		memset(filename, 0, sizeof(filename));
		port = 0;
		recv_buf = "";
		send_buf = "";
		gettimeofday(&send_t, NULL);
		send_t.tv_sec += 1;
	}
	FILE *fp;
	void parse(char *, char *, char *);

	string str_replace(string str, string oldstr, string newstr) {
		int i = str.find(oldstr);
		while (i != -1) {
			str.replace(i, oldstr.length(), newstr);
			i = str.find(oldstr, i + newstr.length());
		}
		return str;
	}
	void js(const char *line) {


		string tmp = line;
		string tmp2 ="";

		while(tmp.length()>200){
			tmp2=tmp.substr(0,200);
			tmp2 = str_replace(tmp2, " ", "&nbsp;");
			tmp2 = str_replace(tmp2, "<", "&lt;");
			tmp2 = str_replace(tmp2, ">", "&gt;");
			tmp2 = str_replace(tmp2, "\r", "");
			tmp2 = str_replace(tmp2, "\n", "<br />");
			tmp2 = str_replace(tmp2, "\"", "\\\"");
			printf("<script type='text/javascript'>document.all['m%d'].innerHTML += \"%s\";</script>\n",
			index, tmp2.c_str());
			fflush(stdout);
			tmp.erase(0,200);
		}


		tmp = str_replace(tmp, " ", "&nbsp;");
		tmp = str_replace(tmp, "<", "&lt;");
		tmp = str_replace(tmp, ">", "&gt;");
		tmp = str_replace(tmp, "\r", "");
		tmp = str_replace(tmp, "\n", "<br />");
		tmp = str_replace(tmp, "\"", "\\\"");

		printf(
				"<script type='text/javascript'>document.all['m%d'].innerHTML += \"%s\";</script>\n",
				index, tmp.c_str());
		fflush(stdout);

		gettimeofday(&time_out,NULL);
		time_out.tv_sec+=7;
	}

	void openfile() {
		try {
			//fflush(stdout);
			//string path=
			if (file_exists(filename)) {
				fp = fopen(filename, "r");
				is_openfile = true;
				readfile();
			} else {
				if (is_con)
					shutdown(con_fd, SHUT_RDWR);
				is_con = false;
				js("script not is exist.\n");
			}
		} catch (...) {

		}
	}
	int readfile() {
		if (is_openfile) {
			char buf[4096];
			bzero(buf, sizeof(buf));
			if (fgets(buf, sizeof(buf), fp) != NULL) {
				//int len = strlen(buf);
				if (is_con)
					send_buf += buf;
				return 1;
			} else {
				is_openfile = false;
				//write_buf = "";
				return 0;
			}
		}

		return 0;
	}
	int senddata() {
		int len = send(con_fd, send_buf.c_str(), send_buf.length(), 0);
		if (len > 0) {
			//cout << write_buf.substr(0, len) << endl;
			string tmp = send_buf.substr(0, len);
			if (is_con)
				js(tmp.c_str());
			send_buf.erase(0, len);
		}
		return len;
	}
	int recvdata() {
		char buf[4096];
		bzero(buf, sizeof(buf));
		int len = recv(con_fd, buf, sizeof(buf), 0);
		if (len > 0) {
			recv_buf += buf;
		} else {
			shutdown(con_fd, SHUT_RDWR);
			is_con = false;
		}

		flushdata();
		return len;
	}

	int flushdata() {
		if (recv_buf.length() > 0) {
			js(recv_buf.c_str());
			recv_buf.erase(0, recv_buf.length());
		}
	}
};

void SerCon::parse(char *shost, char* sport, char *sfilename) {
	strcpy(host, shost);
	strcpy(filename, sfilename);
	port = atoi(sport);
}

SerCon sc[5];
sockaddr_in dest[5];
//map<int, SerCon*> clients;
void connect(int i) {
	try {

		int con_fd = socket(AF_INET, SOCK_STREAM, 0);

		bzero(&dest[i], sizeof(dest[i]));
		dest[i].sin_family = AF_INET;

		struct hostent *server = (struct hostent *) gethostbyname(sc[i].host);
		dest[i].sin_addr.s_addr = inet_addr(sc[i].host);
		dest[i].sin_addr = *((struct in_addr *) server->h_addr);
		//dest[i].sin_addr.s_addr =inet_addr(sc[i].host);
		dest[i].sin_port = htons(sc[i].port);

		if (connect(con_fd, (struct sockaddr *) &dest[i], sizeof(dest[i])) == 0) {
			//cout << "conn "<< i <<sc.host<<sc.port <<sc.filename<<endl;
			//fflush(stdout);
			//clients[sc[i].con_fd] = &sc[i];
			sc[i].con_fd = con_fd;
			sc[i].is_con = true;
			gettimeofday(&sc[i].time_out,NULL);
			sc[i].time_out.tv_sec+=7;
			int flag;
			flag = fcntl(con_fd, F_GETFL, 0);
			fcntl(con_fd, F_SETFL, flag | O_NONBLOCK);

			//cout<<"i="<<i<<con_fd<<endl;
		} else {
			sc[i].js("connect error.\n");
		}
	} catch (...) {
		sc[i].js("connect error.\n");
		//cout << "parse error!";
	}
	/*
	 server = gethostbyname(sc.host);
	 bzero((char *) &sc.serv_addr, sizeof(sc.serv_addr));
	 serv_addr.sin_family = AF_INET;
	 bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, sc.server._length);
	 serv_addr.sin_port = htons(portno);*/
}
void print_table_row() {
	int i = 0;

	printf("<tr>\n");
	for (i = 0; i < 5; i++) {
		bool hasHost = strlen(sc[i].host);
		bool hasFile = strlen(sc[i].filename);
		bool hasPort = sc[i].port != 0;
		//cout<<i<< sc[i].host<<hasHost<<hasFile<<hasPort<<endl;
		if (hasHost && hasFile && hasPort) {
			printf("<td>%s</td>\n", sc[i].host);
		}
	}
	printf("</tr>\n");

	printf("<tr>\n");
	for (i = 0; i < 5; i++) {
		bool hasHost = strlen(sc[i].host);
		bool hasFile = strlen(sc[i].filename);
		bool hasPort = sc[i].port != 0;
		if (hasHost && hasFile && hasPort) {
			printf("<td valign=\"top\" id=\"m%d\"></td>\n", i);
		}
	}
	printf("</tr>");

}
void begin_html() {
	printf("<html>\n"
		"<head>\n"
		"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />\n"
		"</head>\n"
		"<body bgcolor=#336699>\n"
		"<font face=\"Courier New\" size=2 color=#FFFF99>\n"
		"<table width=\"800\" border=\"1\">\n");
	print_table_row();
	printf("</table>");
}
void end_html() {
	printf("</font>\n"
		"</body>\n"
		"</html>\n");
}

void initSc() {
	try {
		sc[0].initobg(0);
		sc[0].parse(&req.h1[0], &req.p1[0], &req.f1[0]);
		sc[1].initobg(1);
		sc[1].parse(&req.h2[0], &req.p2[0], &req.f2[0]);
		sc[2].initobg(2);
		sc[2].parse(&req.h3[0], &req.p3[0], &req.f3[0]);
		sc[3].initobg(3);
		sc[3].parse(&req.h4[0], &req.p4[0], &req.f4[0]);
		sc[4].initobg(4);
		sc[4].parse(&req.h5[0], &req.p5[0], &req.f5[0]);
	} catch (...) {
		//cout << "parse error!";
	}

}
void con_Sc() {
	int i = 0;
	for (i = 0; i < 5; i++) {
		bool hasHost = strlen(sc[i].host);
		bool hasFile = strlen(sc[i].filename);
		bool hasPort = sc[i].port != 0;
		//cout << "conn "<< i <<sc[i].host<<sc[i].port <<sc[i].filename<<endl;
		//fflush(stdout);

		if (hasHost && hasFile && hasPort) {
			connect(i);
			sc[i].openfile();
			//cout << i << endl;
		}

	}
}
void closeAll() {
	fflush(stdout);
	int fd = FD_SETSIZE;
	for (fd; fd > 0; fd--)
		close(fd);
}

void handerSelect() {
	int i = 0;
	for (i = 0; i < 5; i++) {
		if (sc[i].is_con == true) {
			int con_fd = sc[i].con_fd;
			if (FD_ISSET(con_fd, &rfds) || FD_ISSET(con_fd, &wfds)) {
				int error = 0;
				int len = 0;

				int opt = getsockopt(con_fd, SOL_SOCKET, SO_ERROR,
						(void *) &error, (socklen_t*) &len);
				if (opt < 0 || error != 0) {
					// non-blocking connect failed
					FD_CLR(con_fd, &rfds_src);
					FD_CLR(con_fd, &wfds_src);
					sc[i].is_con = false;
					continue;
				}
				if (FD_ISSET(con_fd, &rfds)) {
					int len = sc[i].recvdata();
					sc[i].flushdata();
					if (sc[i].mode < 2)
						sc[i].mode = 1;
					//FD_CLR(con_fd,&rfds_src);
					//FD_SET(con_fd,&wfds_src);
				}

				timeval tv;
				gettimeofday(&tv, NULL);
				if (FD_ISSET(con_fd, &wfds)) {
						if (tv.tv_sec >= sc[i].send_t.tv_sec) {
							sc[i].senddata();
							gettimeofday(&sc[i].send_t, NULL);
							sc[i].send_t.tv_sec += 2;
							if (sc[i].mode < 2)
								sc[i].mode = 0;

							//FD_CLR(con_fd,&wfds_src);
							//FD_SET(con_fd,&rfds_src);
							if (sc[i].send_buf.length() == 0)
								sc[i].readfile();
						}

				}
				gettimeofday(&tv, NULL);
				if ((sc[i].send_buf.length() == 0 && sc[i].recv_buf.length()== 0)
						|| sc[i].is_con == false
						||tv.tv_sec>sc[i].time_out.tv_sec) {
					sc[i].is_con = false;
					sc[i].mode = 2;
					FD_CLR(con_fd, &rfds_src);
					FD_CLR(con_fd, &wfds_src);
					shutdown(con_fd, SHUT_RDWR);
				}

			}

		}

	}

}

void select_con() {
	int i = 0;
	bzero(&rfds_src, sizeof(rfds_src));
	bzero(&wfds_src, sizeof(wfds_src));
	for (i = 0; i < 5; i++) {
		if (sc[i].is_con && sc[i].is_openfile) {
			FD_SET(sc[i].con_fd,&rfds_src);
			FD_SET(sc[i].con_fd,&wfds_src);
		}
	}
	timeval tv;
	tv.tv_sec = 0;
	tv.tv_usec = 1000;
	while (sc[0].is_con || sc[1].is_con || sc[2].is_con || sc[3].is_con
			|| sc[4].is_con) {
		memcpy(&rfds, &rfds_src, sizeof(rfds_src));
		memcpy(&wfds, &wfds_src, sizeof(wfds_src));
		select(64, &rfds, &wfds, (fd_set*) 0, (struct timeval*) &tv);
		handerSelect();
	}

}

int main(int argc, char* argv[], char *envp[]) {
	init_req();

	//char tmp[1024];
	//read(0, tmp, 1024);
	//flag = fcntl(1, F_GETFL, 0);
	//fcntl(1, F_SETFL, flag | O_NONBLOCK);
	// printf("HTTP/1.1 200 OK\n");


	//printf("Accept-Asynchronous: response\n");
	//printf("Content-length:4096\n");
	// printf("Cache-Control: max-age=0\n");
	initSc();
	printf("Content-type: text/html\n");

	// printf("Content-type: text/plain\n");
	printf("\n");
	begin_html();
	fflush(stdout);
	con_Sc();
	select_con();
	end_html();
	closeAll();
	return 0;
}
