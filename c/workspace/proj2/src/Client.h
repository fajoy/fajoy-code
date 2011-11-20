/*
 * Client.h
 *
 *  Created on: 2011/11/13
 *      Author: fajoy
 */

#ifndef CLIENT_H_
#define CLIENT_H_
char ClientDefaultName[]="(No Name)";
typedef struct Client{
	int index;
	int pid;
	char name[255];
	char *remote_ip;
	int remote_port;
	int mainSendPipe[2];
	int clientSendPipe[2];
	int conSocketFd;
}Client;

#endif /* CLIENT_H_ */
