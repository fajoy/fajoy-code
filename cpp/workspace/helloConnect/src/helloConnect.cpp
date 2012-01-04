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
#include <wait.h>
#include <unistd.h>

#include <iostream>
#include <map>
#define debug_fd 1000
/*
#ifdef debug_fd
	dprintf(debug_fd,);
#endif
*/
using namespace std;

class mySelectHelper{
public :
	fd_set rfds;
	fd_set wfds;
	fd_set rfds_src;
	fd_set wfds_src;
	mySelectHelper(){
		memset(&rfds,0,sizeof(fd_set));
		memset(&rfds_src,0,sizeof(fd_set));
		memset(&wfds,0,sizeof(fd_set));
		memset(&wfds_src,0,sizeof(fd_set));
	}
	void reset(){
		memcpy(&rfds,&rfds_src,sizeof(fd_set));
		memcpy(&wfds,&wfds_src,sizeof(fd_set));
	}
	int select_fd(int nfds){
			reset();
			return select(nfds,&rfds,&wfds,(fd_set *)0,(timeval*)0);
	}

	int isRSet(int fd){
		return FD_ISSET(fd,&rfds);
	}
	int isWSet(int fd){
		return FD_ISSET(fd,&wfds);
	}
	int setR(int fd){
			return FD_SET(fd,&rfds_src);
	}
	int setW(int fd){
			return FD_SET(fd,&wfds_src);
	}
	int clrR(int fd){
			return FD_CLR(fd,&rfds_src);
	}
	int clrW(int fd){
			return FD_CLR(fd,&wfds_src);
	}


};

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
		if (socket_fd != -1){
				re = read(socket_fd, &recv_buffer[recv_len], sizeof(recv_buffer)-recv_len);
			if (re < 0) {
				recvError = true;
			} else if (re > 0) {
				recv_len+=re;
				hasRecvData = true;
			}
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
			if(status==0)
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
			status=0;
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
mySelectHelper fds;

mySocket server;
mySocket client;

void clientAction(){
	sleep(1);
	server.close_listen();
	cout <<"start connect."<<endl;
	client.remote_host="localhost";
	client.remote_port=server.local_port;
	client.creatSocket();

	//sleep(1);
	client.connect_socket();
	fds.setR(client.socket_fd);
	fds.setW(client.socket_fd);
	while(1){
		client.setNonblock();
		cout <<"client start select ."<<endl;
		fflush(stdout);
		fds.select_fd(1023);

		if(client.getMode()==-2&&(fds.isRSet(client.socket_fd)||fds.isWSet(client.socket_fd))){
			if(client.isConnectError())
				break;
			fds.clrR(client.socket_fd);

		}else
		if(fds.isWSet(client.socket_fd)){
			cout <<"start send."<<endl;
			fflush(stdout);
			unsigned char data[]="test\n";
			client.setSendData(&data[0],5);
			client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);
			client.sendData();
			client.setSendData(&data[0],5);
			client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);client.setSendData(&data[0],5);
			client.sendData();
			if(client.hasSendData){

			}else{
				cout <<"client send len:"<<client.send_len<<endl;
				fflush(stdout);
				client.setMode(0);
				fds.clrW(client.socket_fd);
				break;
			}
		}else
			if(fds.isRSet(client.socket_fd)){
				client.recvData();
				if(client.hasRecvData){
					cout <<client.recv_buffer<<endl;
					fflush(stdout);
				}else{
					client.setMode(-1);
					break;
				}
		}

	}

	client.close_socket();
	cout <<"client end."<<endl;
	fflush(stdout);
	exit(EXIT_SUCCESS);
}
void serverAction(){
	cout <<"start listen."<<endl;
	fflush(stdout);
	fds.setR(server.listen_fd);

	while(1){
		fds.select_fd(1023);
		if(fds.isRSet(server.listen_fd)){
			server.accept_socket();
			cout <<"accept."<<endl;
			fflush(stdout);
			server.setNonblock();
			fds.setR(server.socket_fd);
		}

		if(fds.isRSet(server.socket_fd)){
			server.recvData();
			if(server.hasRecvData){
			cout <<"server rev len:"<<server.recv_len<<"\n"<<server.recv_buffer;
			fflush(stdout);
			}else{
				break;
			}
		}

	}

	server.close_socket();
	server.close_listen();
	cout <<"server end."<<endl;
	fflush(stdout);
	exit(EXIT_SUCCESS);


}

int main() {
#ifdef debug_fd
	dup2(1,debug_fd);
#endif
	/*
	mySocket *client=new mySocket();
	client->remote_host="ftp.myweb.hinet.net";
	client->remote_port=21;
	cout<<client->connect_socket()<<endl;
	fflush(stdout);
	cout <<client->socket_fd<<endl;
	client->recvdata();
	cout <<client->recv_buffer<<endl;
	fflush(stdout);
	client->recvdata();
	cout <<client->recv_buffer<<endl;
	fflush(stdout);
	client->close();
	client->~mySocket();
	//client->close();
	free(client);
*/
	server=mySocket();
	server.local_port=7000;
	server.listen_socket();

	cout << server.local_port <<endl;
	fflush(stdout);
	int pid=0;
	pid=fork();
	if(pid==0){
		clientAction();
		return 0;
	}else{
		serverAction();
	}

	return 0;
}
