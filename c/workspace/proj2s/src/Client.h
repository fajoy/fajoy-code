/*
 * Client.h
 *
 *  Created on: 2011/11/13
 *      Author: fajoy
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#define queMAX 32767
char ClientDefaultName[]="(no name)";
typedef struct Client{
    char PATH[255];
	int index;
	int pid;
	char name[255];
	char remote_ip[255];
	int remote_port;
	int mainSendPipe[2];
	int clientSendPipe[2];
	int conSocketFd;
	int fifo_fd[2];
	char fifo_path[255];
	char cmd_buffer[1024];
	char read_buffer[1024];
	char *read_buffer_w;
	char *read_buffer_r;
	PipeFD queFd[queMAX];
	PipeFD console;
	Cmd cmd;
	int qi;
        struct sockaddr_in dest;
        struct sockaddr_in serv;
        int erro_fd;
        int has_err_pipe ;
int has_fifoOut ;
int has_fifoIn ;
int fifoInNumber;

 int mainPid ;
 char cmd_src[255];
 int waitPid;
 int listenCmd;
}Client;

#endif /* CLIENT_H_ */
