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
	char sh1[255];
	char sh2[255];
	char sh3[255];
	char sh4[255];
	char sh5[255];
	char sp1[5];
	char sp2[5];
	char sp3[5];
	char sp4[5];
	char sp5[5];

} reqdata;
char *query;
reqdata req;
void init_req() {
	query = getenv("QUERY_STRING");

	if (query == NULL) {
		char
				testquery[] =
						"h1=ubu.fajoy.co.cc&p1=8001&f1=t1.txt&sh1=ubu.fajoy.co.cc&sp1=8000&h2=ubu.fajoy.co.cc&p2=8001&f2=t1.txt&sh2=ubu.fajoy.co.cc&sp2=8000&h3=ubu.fajoy.co.cc&p3=8001&f3=t1.txt&sh3=ubu.fajoy.co.cc&sp3=8000&h4=ubu.fajoy.co.cc&p4=8001&f4=t1.txt&sh4=ubu.fajoy.co.cc&sp4=8000&h5=ubu.fajoy.co.cc&p5=8001&f5=t1.txt&sh5=ubu.fajoy.co.cc&sp5=8000";
		query = testquery;
	}
	memset(&req, 0, sizeof(req));
	sscanf(query, "h1=%[^&]&p1=%[^&]&f1=%[^&]&sh1=%[^&]&sp1=%[^&]&"
		"h2=%[^&]&p2=%[^&]&f2=%[^&]&sh2=%[^&]&sp2=%[^&]&"
		"h3=%[^&]&p3=%[^&]&f3=%[^&]&sh3=%[^&]&sp3=%[^&]&"
		"h4=%[^&]&p4=%[^&]&f4=%[^&]&sh4=%[^&]&sp4=%[^&]&"
		"h5=%[^&]&p5=%[^&]&f5=%[^&]&sh5=%[^&]&sp5=%[^&]", &req.h1[0],
			&req.p1[0], &req.f1[0], &req.sh1[0], &req.sp1[0], &req.h2[0],
			&req.p2[0], &req.f2[0], &req.sh2[0], &req.sp2[0], &req.h3[0],
			&req.p3[0], &req.f3[0], &req.sh3[0], &req.sp3[0], &req.h4[0],
			&req.p4[0], &req.f4[0], &req.sh4[0], &req.sp4[0], &req.h5[0],
			&req.p5[0], &req.f5[0], &req.sh5[0], &req.sp5[0]);

}
//client request conect
//0: 4(4) VC
//1: 1(1)
//2: 0(0) DST PORT
//3:50(80)
//4:C0(192) DST IP
//5:A8(168)
//6:89(137)
//7:64(100)
//8:4D(77) USER ID
//9:4F(79)
//10:5A(90)
//11: 0(0)
class mySelectHelper {
public:
	fd_set rfds;
	fd_set wfds;
	fd_set rfds_src;
	fd_set wfds_src;
	mySelectHelper() {
		memset(&rfds, 0, sizeof(fd_set));
		memset(&rfds_src, 0, sizeof(fd_set));
		memset(&wfds, 0, sizeof(fd_set));
		memset(&wfds_src, 0, sizeof(fd_set));
	}
	void reset() {
		memcpy(&rfds, &rfds_src, sizeof(fd_set));
		memcpy(&wfds, &wfds_src, sizeof(fd_set));
	}
	int select_fd(int nfds) {
		reset();
		return select(nfds, &rfds, &wfds, (fd_set *) 0, (timeval*) 0);
	}

	int isRSet(int fd) {
		return FD_ISSET(fd,&rfds);
	}
	int isWSet(int fd) {
		return FD_ISSET(fd,&wfds);
	}
	int setR(int fd) {
		return FD_SET(fd,&rfds_src);
	}
	int setW(int fd) {
		return FD_SET(fd,&wfds_src);
	}
	int clrR(int fd) {
		return FD_CLR(fd,&rfds_src);
	}
	int clrW(int fd) {
		return FD_CLR(fd,&wfds_src);
	}

};
mySelectHelper fds;
class mySocket {
public:

	sockaddr_in local_addr;
	sockaddr_in remote_addr;
	string local_host;
	string remote_host;
	int local_port;
	int remote_port;
	int listen_fd;
	int socket_fd;
	int status;
	unsigned char recv_buffer[1024];
	unsigned char send_buffer[1024];
	unsigned char *sendp;
	unsigned char buffer[1024];
	//Read N bytes into BUF from socket FD.
	//Returns the number read or -1 for errors.
	int recv_len;

	int send_len;
	bool hasSendData;
	bool hasRecvData;
	bool sendError;
	bool recvError;
	mySocket() {
		socket_fd = -1;
		listen_fd = -1;
		recv_len = 0;
		send_len = 0;
		status = -1;
		clear_send_data();
		clear_recv_data();
	}
	bool isConnectError() {
		int error = 0;
		int len = 0;
		int opt = getsockopt(socket_fd, SOL_SOCKET, SO_ERROR, (void *) &error,
				(socklen_t*) &len);
		if (opt < 0 || error != 0) {
			// non-blocking connect failed
			return true;
		}
		return false;

	}
	void setNonblock() {
		if (socket_fd > 0) {
			int flag;
			flag = fcntl(socket_fd, F_GETFL, 0);
			fcntl(socket_fd, F_SETFL, flag | O_NONBLOCK);
		}
	}
	int getStat() {
		if (socket_fd > 0) {
			int flag;
			flag = fcntl(socket_fd, F_GETFL, 0);
			return flag;
		}
	}

	void close_listen() {
		if (listen_fd != -1) {
			//shutdown(listen_fd,SHUT_WR);
			close(listen_fd);
			listen_fd = -1;
		}
	}
	void close_socket() {
		if (socket_fd != -1) {
			//shutdown(socket_fd,SHUT_WR);
			close(socket_fd);
			socket_fd = -1;
		}
	}

	void clear_recv_data() {
		memset(recv_buffer, 0, sizeof(recv_buffer));
		recv_len = 0;
		hasRecvData = false;
		recvError = false;

	}
	void paasiveTcp1() {
		//int fd=paasiveTcp

	}
	bool recvData(bool clearRecvBuffer) {
		if (clearRecvBuffer)
			clear_recv_data();

		int re = 0;
		if (socket_fd != -1) {
			re = read(socket_fd, &recv_buffer[recv_len], sizeof(recv_buffer)
					- recv_len);
			if (re < 0) {
				recvError = true;
			} else if (re > 0) {
				recv_len += re;
				hasRecvData = true;
			}
			//cout<<re <<endl;
			//fflush(stdout);
		}

		return hasRecvData;
	}
	bool recvData() {
		return recvData(true);
	}
	void clear_send_data() {
		memset(send_buffer, 0, sizeof(send_buffer));
		send_len = 0;
		hasSendData = false;
		sendError = false;
	}
	void setSendData(unsigned char *data, int size) {
		if (status < 0) {
			clear_send_data();
			return;
		}
		if (size > 0) {
			memcpy(&send_buffer[send_len], data, size);
			send_len += size;
			if (status == 0)
				status = 1;
			hasSendData = true;
		}
	}
	void sendData() {
		if (hasSendData) {
			int re_send = -1;
			if (socket_fd != -1)
				re_send = write(socket_fd, &send_buffer[0], send_len);
			if (re_send < 0) {
				sendError = true;
				//clear_send_data();
			} else if (re_send > 0) {
				if (re_send != send_len) {
					sendp = &send_buffer[re_send];
					memset(buffer, 0, sizeof(send_buffer));
					memcpy(buffer, sendp, send_len - re_send);
					memset(send_buffer, 0, sizeof(send_buffer));
					memcpy(send_buffer, buffer, send_len - re_send);
					send_len -= re_send;
					//cout << re_send << "re!=se" << send_len << endl;
					//fflush(stdout);

				} else {
					clear_send_data();
					setMode(0);
				}
			}
		}
	}

	void sendData(unsigned char *data, int size) {
		setSendData(data, size);
		sendData();
	}

	int creatSocket() {
		socket_fd = socket(AF_INET, SOCK_STREAM, 0);
		return socket_fd;
	}
	//Return 0 on success, -1 for errors.
	int connect_socket() {
		if (socket_fd == -1)
			creatSocket();
		status = -2;
		memset(&remote_addr, 0, sizeof(sockaddr_in));
		remote_addr.sin_family = AF_INET;
		struct hostent *host = (struct hostent *) gethostbyname(
				remote_host.c_str());
		remote_addr.sin_addr = *((struct in_addr *) host->h_addr);
		remote_addr.sin_port = htons(remote_port);
		//remote_addr.sin_addr.s_addr =inet_addr(host.c_str());
		int re = connect(socket_fd, (struct sockaddr *) &remote_addr,
				sizeof(struct sockaddr_in));
		if (re == 0)
			status = 0;
		return re;
	}
	//Returns 0 on success, -1 for errors.
	int listen_socket() {
		listen_fd = socket(AF_INET, SOCK_STREAM, 0);
		memset(&local_addr, 0, sizeof(sockaddr_in)); /* zero the struct before filling the fields */
		local_addr.sin_family = AF_INET; /* set the type of connection to TCP/IP */
		local_addr.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
		local_addr.sin_port = htons(local_port); /* set the server port number */
		bind(listen_fd, (struct sockaddr *) &local_addr,
				sizeof(struct sockaddr));
		return listen(listen_fd, 1);
	}
	//return accept -1=error
	int accept_socket() {
		memset(&remote_addr, 0, sizeof(sockaddr_in));
		socklen_t len = sizeof(struct sockaddr_in);
		socket_fd = accept(listen_fd, (struct sockaddr *) &remote_addr, &len);
		if (socket_fd > 0) {
			char *addr = inet_ntoa(remote_addr.sin_addr);
			char tmp[17];
			memset(tmp, 0, 17);
			memcpy(tmp, addr, 17);
			remote_host = tmp;
			remote_port = ntohs(remote_addr.sin_port);
			status = 0;
		}
		return socket_fd;
	}
	void setMode(int s) {
		status = s;
	}
	int getMode() {
		return status;
	}

};
typedef struct SocksData {
	unsigned char VN;
	unsigned char CD;
	unsigned char port[2];
	unsigned char ip[4];
	unsigned char userid[300];
} SocksData;

class SerCon {
public:
	int index;
	char host[255];
	char socks[255];
	int sport;
	int port;
	char filename[255];
	int con_fd;
	bool is_con;
	bool is_openfile;
	string recv_buf;
	string send_buf;
	int mode;

	timeval time_out;
	SocksData sd;
	mySocket s;
	SerCon() {

	}
	void connectSocks() {

	}

	void sendSocksData() {
		int len = send(con_fd, &sd, 9, 0);
	}
	void recvSocksData() {
		char buf[255];
		bzero(buf, sizeof(buf));
		unsigned char b = 0;
		int len = 0;
		unsigned char *p = (unsigned char *) &sd;
		len = recv(con_fd, p, 9, 0);
		//p += 8;
		//cout << len<<endl;
		//fflush(stdout);

		/*
		 while(len = recv(con_fd,p ,1 , 0)){
		 if(len<0)
		 break;
		 if(*p>0){
		 p++;
		 }else{
		 break;
		 }
		 }*/
	}
	void checkSocksData() {


		if (sd.CD == 2) {
			js("cd=2\n");
		} else {

		}
	}

	void setSocksData() {
		sd.VN = 4;
		sd.CD = 1;
		sd.port[0] = port / 256;
		sd.port[1] = port % 256;
		struct hostent *server = (struct hostent *) gethostbyname(host);
		if (server) {
			bcopy(*server->h_addr_list, &sd.ip[0], 4);
		}
		sd.userid[0] = 0;
		//int ip=0;
		//if
		//ip[0]=
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
		s = mySocket();
		index = i;
		mode = 1;
		is_openfile = false;
		is_con = false;
		memset(host, 0, sizeof(host));
		memset(socks, 0, sizeof(socks));
		memset(filename, 0, sizeof(filename));
		port = 0;
		recv_buf = "";
		send_buf = "";

	}
	FILE *fp;
	void parse(char *shost, char* str_port, char *sfilename, char* ssocks,
			char * ssport) {
		strcpy(host, shost);
		strcpy(socks, ssocks);
		strcpy(filename, sfilename);
		port = atoi(str_port);
		sport = atoi(ssport);
	}
	string str_replace(string str, string oldstr, string newstr) {
		int i = str.find(oldstr);
		while (i != -1) {
			str.replace(i, oldstr.length(), newstr);
			i = str.find(oldstr, i + newstr.length());
		}
		return str;
	}
	void js(char *line) {

		string tmp = line;
		string tmp2 = "";

		while (tmp.length() > 200) {
			tmp2 = tmp.substr(0, 200);
			tmp2 = str_replace(tmp2, " ", "&nbsp;");
			tmp2 = str_replace(tmp2, "<", "&lt;");
			tmp2 = str_replace(tmp2, ">", "&gt;");
			tmp2 = str_replace(tmp2, "\r", "");
			tmp2 = str_replace(tmp2, "\n", "<br />");
			tmp2 = str_replace(tmp2, "\"", "\\\"");
			printf(
					"<script type='text/javascript'>document.all['m%d'].innerHTML += \"%s\";</script>\n",
					index, tmp2.c_str());
			fflush(stdout);
			tmp.erase(0, 200);
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

	}
	void bjs(char *line) {

		string tmp = line;
		string tmp2 = "";

		while (tmp.length() > 200) {
			tmp2 = tmp.substr(0, 200);
			tmp2 = str_replace(tmp2, " ", "&nbsp;");
			tmp2 = str_replace(tmp2, "<", "&lt;");
			tmp2 = str_replace(tmp2, ">", "&gt;");
			tmp2 = str_replace(tmp2, "\r", "");
			tmp2 = str_replace(tmp2, "\n", "<br />");
			tmp2 = str_replace(tmp2, "\"", "\\\"");
			printf(
					"<script type='text/javascript'>document.all['m%d'].innerHTML += \"<b>%s</b>\";</b></script>\n",
					index, tmp2.c_str());
			fflush(stdout);
			tmp.erase(0, 200);
		}

		tmp = str_replace(tmp, " ", "&nbsp;");
		tmp = str_replace(tmp, "<", "&lt;");
		tmp = str_replace(tmp, ">", "&gt;");
		tmp = str_replace(tmp, "\r", "");
		tmp = str_replace(tmp, "\n", "<br />");
		tmp = str_replace(tmp, "\"", "\\\"");

		printf(
				"<script type='text/javascript'>document.all['m%d'].innerHTML += \"<b>%s</b>\";</script>\n",
				index, tmp.c_str());
		fflush(stdout);


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
			send_buf.clear();
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
	timeval send_t;
	void setSendtime(){
		gettimeofday(&send_t, NULL);
		send_t.tv_sec += 0;
	}
	bool isCanSend(){
		timeval now;
		gettimeofday(&now, NULL);
		if(now.tv_sec>send_t.tv_sec){
			return true;
		}
		return false;
	}

};

SerCon sc[5];
sockaddr_in dest[5];

void connect(SerCon *c) {
	try {
		c->s.remote_host = c->socks;
		c->s.remote_port = c->sport;
		c->s.creatSocket();
		if (c->s.connect_socket() == 0) {
			//c->is_con
			//c->n
			c->con_fd = c->s.socket_fd;
			c->s.setNonblock();
			fds.setR(c->s.socket_fd);
			c->setSocksData();
			c->sendSocksData();

			fds.select_fd(1023);
			fds.clrR(c->s.socket_fd);
			c->recvSocksData();
			c->checkSocksData();
			c->is_con = true;
			c->js("connect ok.\n");
		} else {
			c->js("connect error.\n");

		}
		////                int con_fd = socket(AF_INET, SOCK_STREAM, 0);
		////
		////                bzero(&dest[i], sizeof(dest[i]));
		////                dest[i].sin_family = AF_INET;
		////
		////                struct hostent *server = (struct hostent *) gethostbyname(sc[i].host);
		////                dest[i].sin_addr = *((struct in_addr *) server->h_addr);
		////                dest[i].sin_addr.s_addr = inet_addr(sc[i].host);
		////                //dest[i].sin_addr.s_addr =inet_addr(sc[i].host);
		////                dest[i].sin_port = htons(sc[i].port);
		//
		//                if (connect(con_fd, (struct sockaddr *) &dest[i], sizeof(dest[i])) == 0) {
		//                        //cout << "conn "<< i <<sc.host<<sc.port <<sc.filename<<endl;
		//                        //fflush(stdout);
		//                        //clients[sc[i].con_fd] = &sc[i];
		//                        sc[i].con_fd = con_fd;
		//                        sc[i].is_con = true;
		//                        gettimeofday(&sc[i].time_out,NULL);
		//                        sc[i].time_out.tv_sec+=7;
		//                        int flag;
		//                        flag = fcntl(con_fd, F_GETFL, 0);
		//                        fcntl(con_fd, F_SETFL, flag | O_NONBLOCK);
		//
		//                        //cout<<"i="<<i<<con_fd<<endl;
		//                } else {
		//                        sc[i].js("connect error.\n");
		//                }
	} catch (...) {
		c->js("connect error.\n");
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
			//printf("<td>%s</td>\n", sc[i].host);
			printf("<td>%s %d %s %d</td>\n", sc[i].host,sc[i].port,sc[i].socks,sc[i].sport);//my_debug
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
		sc[0].parse(&req.h1[0], &req.p1[0], &req.f1[0], &req.sh1[0],
				&req.sp1[0]);
		sc[1].initobg(1);
		sc[1].parse(&req.h2[0], &req.p2[0], &req.f2[0], &req.sh2[0],
				&req.sp2[0]);
		sc[2].initobg(2);
		sc[2].parse(&req.h3[0], &req.p3[0], &req.f3[0], &req.sh3[0],
				&req.sp3[0]);
		sc[3].initobg(3);
		sc[3].parse(&req.h4[0], &req.p4[0], &req.f4[0], &req.sh4[0],
				&req.sp4[0]);
		sc[4].initobg(4);
		sc[4].parse(&req.h5[0], &req.p5[0], &req.f5[0], &req.sh5[0],
				&req.sp5[0]);

	} catch (...) {
		//cout << "parse error!";
	}

}
void con_Sc() {
	int i = 0;
	for (i = 0; i < 5; i++) {
		bool hasHost = strlen(sc[i].host)&&strlen(sc[i].socks);
		bool hasFile = strlen(sc[i].filename);
		bool hasPort = sc[i].port&&sc[i].sport != 0;
		//cout << "conn "<< i <<sc[i].host<<sc[i].port <<sc[i].filename<<endl;
		//fflush(stdout);

		if (hasHost && hasFile && hasPort) {
			connect(&sc[i]);
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

//void handerSelect() {
//        int i = 0;
//        for (i = 0; i < 5; i++) {
//                if (sc[i].is_con == true) {
//                        int con_fd = sc[i].con_fd;
//                        if (FD_ISSET(con_fd, &rfds) || FD_ISSET(con_fd, &wfds)) {
//                                int error = 0;
//                                int len = 0;
//
//                                int opt = getsockopt(con_fd, SOL_SOCKET, SO_ERROR,
//                                                (void *) &error, (socklen_t*) &len);
//                                if (opt < 0 || error != 0) {
//                                        // non-blocking connect failed
//                                        FD_CLR(con_fd, &rfds_src);
//                                        FD_CLR(con_fd, &wfds_src);
//                                        sc[i].is_con = false;
//                                        continue;
//                                }
//                                if (FD_ISSET(con_fd, &rfds)) {
//                                        int len = sc[i].recvdata();
//                                        sc[i].flushdata();
//                                        if (sc[i].mode < 2)
//                                                sc[i].mode = 1;
//                                        //FD_CLR(con_fd,&rfds_src);
//                                        //FD_SET(con_fd,&wfds_src);
//                                }
//
//                                timeval tv;
//                                gettimeofday(&tv, NULL);
//                                if (FD_ISSET(con_fd, &wfds)) {
//                                                if (tv.tv_sec >= sc[i].send_t.tv_sec) {
//                                                        sc[i].senddata();
//                                                        gettimeofday(&sc[i].send_t, NULL);
//                                                        sc[i].send_t.tv_sec += 2;
//                                                        if (sc[i].mode < 2)
//                                                                sc[i].mode = 0;
//
//                                                        //FD_CLR(con_fd,&wfds_src);
//                                                        //FD_SET(con_fd,&rfds_src);
//                                                        if (sc[i].send_buf.length() == 0)
//                                                                sc[i].readfile();
//                                                }
//
//                                }
//                                gettimeofday(&tv, NULL);
//                                if ((sc[i].send_buf.length() == 0 && sc[i].recv_buf.length()== 0)
//                                                || sc[i].is_con == false
//                                                ||tv.tv_sec>sc[i].time_out.tv_sec) {
//                                        sc[i].is_con = false;
//                                        sc[i].mode = 2;
//                                        FD_CLR(con_fd, &rfds_src);
//                                        FD_CLR(con_fd, &wfds_src);
//                                        shutdown(con_fd, SHUT_RDWR);
//                                }
//
//                        }
//
//                }
//
//        }
//
//}

void select_con() {
	int i = 0;
	int con=0;

	for (i = 0; i < 5; i++) {
		if (sc[i].is_con && sc[i].is_openfile) {
			fds.setR(sc[i].s.socket_fd);
			//fds.isSet(sc[i].s.socket_fd);
			//printf("%d\n",sc[i].s.socket_fd);
			//fflush(stdout);

			sc[i].s.setMode(0);
			con++;
			sc[i].setSendtime();
			//FD_SET(sc[i].con_fd,&rfds_src);
			//FD_SET(sc[i].con_fd,&wfds_src);
		}
	}

	while (con > 0) {

		fds.select_fd(1023);

		for (i = 0; i < 5; i++) {
			SerCon *c = &sc[i];
			mySocket *s = &sc[i].s;
			//cout <<"con:"<<con<<i<<" s:"<<s->getMode()
			//<<"s"<<s->send_len<<"r"<<s->recv_len<<endl;
			if (fds.isWSet(s->socket_fd) && s->getMode() == 1&&c->isCanSend()) {
				s->sendData();
				if (s->hasSendData) {

					if (s->sendError) {
						fds.clrW(s->socket_fd);
						fds.clrR(s->socket_fd);
						s->setMode(-1);
						con--;
					}
				} else {
					s->setMode(0);
					fds.clrW(s->socket_fd);
					//fds.clrR(s->socket_fd);

				}

			} else if (fds.isRSet(s->socket_fd) ) {
				s->recvData(false);
				if (s->hasRecvData) {
							c->js((char *)&s->recv_buffer[0]);
							s->clear_recv_data();
							if(s->send_len==0){
							c->readfile();
							s->setSendData((unsigned char *)c->send_buf.c_str(),c->send_buf.length());
							c->bjs(&c->send_buf[0]);
							}
							fds.setW(s->socket_fd);
							//fds.clrR(s->socket_fd);
							s->setMode(1);
							c->setSendtime();
				} else {
					con--;
					fds.clrR(s->socket_fd);
					s->setMode(-1);
				}
			}
			//cout <<"con:"<<con<<" i:"<<i<<" s:"<<s->getMode()
					//<<"s"<<s->send_len<<"r"<<s->recv_len<<endl;
		}

	}
	for (i = 0; i < 5; i++) {
		if (sc[i].is_con && sc[i].is_openfile) {
			sc[i].s.close_socket();
			//FD_SET(sc[i].con_fd,&rfds_src);
			//FD_SET(sc[i].con_fd,&wfds_src);
		}
	}

}

int main(int argc, char* argv[], char *envp[]) {
	fds = mySelectHelper();
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
