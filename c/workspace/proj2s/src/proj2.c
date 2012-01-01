#define PIPE_BUF 4096
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/select.h>

#include <errno.h>
#include "Array.h"
#define debug
int PORTNUM = 8000;
int stdo_fd;
int stdi_fd;

typedef struct PipeFD {
	int out_fd;
	int in_fd;
} PipeFD;
typedef struct Cmd {
	int p_id;
	int wait;
	int argc;
	char **argv;
	int readFd;
	int writeFd;
	PipeFD outPipe;
	struct Cmd *parent;
	struct Cmd *first;
	struct Cmd *prev;
	struct Cmd *next;
} Cmd;

PipeFD newPipeFD() {
	int fd[2];
	pipe(fd);
	PipeFD pfd = { fd[0], fd[1] };
	return pfd;
}

#include "ClientQueue.h"

fd_set main_rfds_src;
void fifoPipe(int * fd, char *fifo_path) {
	int status = mkfifo(fifo_path, 0666);
	int tmp_out = open(fifo_path, O_RDONLY | O_NONBLOCK);
	int tmp_in = open(fifo_path, O_WRONLY);
	*fd = tmp_out;
	fd += 1;
	*fd = tmp_in;
}

static void whoCmd(int writeFd, int index) {

	int i;
	int pid = getpid();
	dprintf(writeFd, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
	for (i = 0; i < ClientQueueLength; i++) {
		Client *c = &clientQueue[i];
		char isme[] = "<-me";
		if (c->pid > 0) {
			if (i == index) {
				dprintf(writeFd, "%d\t%s\t%s/%d\t%s\n", c->index + 1,
						c->name, c->remote_ip, c->remote_port, isme);
			} else {
				dprintf(writeFd, "%d\t%s\t%s/%d\t\n", c->index + 1, c->name,
						c->remote_ip, c->remote_port);
			}
		}
	}
}
static void clientCastMsg(char *msg, int selfIndex) {
	int i;
	for (i = 0; i < ClientQueueLength; i++) {
		Client *c = &clientQueue[i];
		if (c->pid > 0) {
			if (selfIndex != i) {
				if (strlen(msg) > 0)
					dprintf(c->conSocketFd, "%s", msg);
			}
		}
	}

}

Array *split(char *str, const char *del) {
	char *root = strdup(str);
	char *sub = strtok(root, del);
	Array *list = newArray(100);
	int i;
	while (sub != NULL) {
		i = list->length;
		ArrayPush(list, sub);
		sub = strtok(NULL, del);
	}
	free(sub);
	return list;
}

void closePipe(PipeFD *fd) {
	if (fd->in_fd > 2)
		close(fd->in_fd);
	if (fd->out_fd > 2)
		close(fd->out_fd);
	fd->in_fd = -1;
	fd->out_fd = -1;
}
void closeOutput(PipeFD *fd) {
#ifdef debug
	dprintf(stdo_fd, "(%d)closeOuput(%d)  ", getpid(), fd->out_fd);
#endif

	if (fd->out_fd > 2)
		close(fd->out_fd);
	fd->out_fd = -1;
}
void closeInput(PipeFD *fd) {
#ifdef debug
	dprintf(stdo_fd, "(%d)closeInput(%d)  ", getpid(), fd->in_fd);
#endif
	if (fd->out_fd > 2)
		close(fd->in_fd);
	fd->out_fd = -1;
}
int isNull(PipeFD *fd) {
	return (fd->in_fd <= 0 && fd->out_fd <= 0);
}
Cmd getCmd(char *cmd, char *del) {
	Cmd c;
	Array *arr = split(cmd, del);
	if (arr->length != 0)
		c.argc = ToArray(arr, &c.argv);
	else
		c.argc = 0;
	c.parent = NULL;
	c.prev = NULL;
	c.next = NULL;
	c.outPipe.in_fd = -1;
	c.outPipe.out_fd = -1;
	c.writeFd = -1;
	c.readFd = -1;
	c.wait = 0;
	return c;
}

int getNullFd() {
	return open("/dev/null", O_RDWR);
}
static void closeAllFd() {
	int i = 0;
	for (i = FD_SETSIZE - 1; i >= 0; i--) {
		close(i);
	}
}

void setCmd(Client *client, Cmd *cmd) {
	int i, fd = 1;

	int has_fifoOut = client->has_fifoOut;
	int has_err_pipe = client->has_err_pipe;
	int has_fifoIn = client->has_fifoIn;
	int fifoInNumber = client->fifoInNumber;
	char *cmd_src = client->cmd_src;
	for (i = 1; i < cmd->argc; i++) {
		if (strncmp(cmd->argv[i], ">", 1) == 0) {

			if (has_fifoOut > 0) {

				//dprintf(1,"user exist\n");
				if (client ->fifo_fd[1] == -1) {

					client->has_err_pipe = indexOf(cmd_src, ">!");

					//fifoPipe(client->fifo_fd, client->fifo_path);

					client->fifo_fd[1] = open(client->fifo_path, O_WRONLY
							| O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP
							| S_IWGRP);

					cmd->writeFd = client->fifo_fd[1];
					char msg[255];
					memset(msg, 0, sizeof(msg));
					sprintf(
							msg,
							"*** %s (#%d) just piped '%s' into his/her pipe. ***\n",
							client->name, client->index + 1, cmd_src);
					clientCastMsg(msg, -1);
					//write(client->clientSendPipe[1], fifocmd, strlen(fifocmd));
				} else {
					dprintf(client->conSocketFd,
							"*** Error: your pipe already exists. ***\n");
					cmd->writeFd = getNullFd();
				}

				cmd->argv[i] = NULL;
				i++;
			}

			else {
				char *fpath = cmd->argv[i + 1];
				if (strncmp(cmd->argv[i], ">>", 2) == 0)
					fd = open(fpath, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR
							| S_IWUSR | S_IRGRP | S_IWGRP);
				else
					fd = open(fpath, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR
							| S_IWUSR | S_IRGRP | S_IWGRP);
				cmd->argv[i] = NULL;
				cmd->writeFd = fd;
				i++;
				cmd->argv[i] = NULL;
			}

		} else if (strncmp(cmd->argv[i], "<", 1) == 0) {
			cmd->argv[i] = NULL;
			if (has_fifoIn > 0) {

				int clientIndex = fifoInNumber - 1;
				if (clientIndex >= 0 && clientIndex < ClientQueueLength) {

					Client *c = &clientQueue[clientIndex];
					if (c->pid > 0) {
						//if (c->fifo_fd[0] > 0) {
						if (c->fifo_fd[1] > 0) {

							int fd[2];
							//fifoPipe((int *)&fd, c->fifo_path);

							fd[0] = open(c->fifo_path, O_RDONLY, S_IRUSR
									| S_IWUSR | S_IRGRP | S_IWGRP);
							c->fifo_fd[1] = -1;

							cmd->readFd = fd[0];
							char msg[255];
							memset(msg, 0, sizeof(msg));
							sprintf(
									msg,
									"*** %s (#%d) just received the pipe from %s (#%d) by '%s' ***\n",
									client->name, client->index + 1, c->name,
									fifoInNumber, client->cmd_src);
							clientCastMsg(msg, -1);

							//close(fd[1]);
						} else {
							dprintf(
									client->conSocketFd,
									"*** Error: the pipe from #%d does not exist yet. ***\n",
									fifoInNumber);
							cmd->readFd = getNullFd();
							cmd->writeFd = getNullFd();

						}
					} else {
						dprintf(
								client->conSocketFd,
								"*** Error: the pipe from #%d does not exist yet. ***\n",
								fifoInNumber);
						cmd->readFd = getNullFd();
						cmd->writeFd = getNullFd();
					}

				}

				else {
					dprintf(
							client->conSocketFd,
							"*** Error: the pipe from #%d does not exist yet. ***\n",
							fifoInNumber);
					cmd->readFd = getNullFd();
					cmd->writeFd = getNullFd();
				}

				cmd->argv[i] = NULL;
			} else {
				char *fpath = cmd->argv[i + 1];
				fd = open(fpath, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP
						| S_IWGRP);
				dprintf(stdo_fd, "in%s\n", cmd->argv[i]);
				cmd->readFd = fd;
				cmd->argv[i] = NULL;
				i++;
				cmd->argv[i] = NULL;

			}

		}

	}

	if (cmd->readFd == -1) {
		if (isNull(&cmd->outPipe)) {
			cmd->readFd = client->conSocketFd;

		} else {
			cmd->readFd = cmd->outPipe.out_fd;
		}
	}

	if (cmd->writeFd == -1) {
		if (cmd->next == NULL) {
			cmd->writeFd = client->conSocketFd;
			cmd->wait = 1;
		} else {
			if (isNull(&cmd->next->outPipe))
				cmd->next->outPipe = newPipeFD();
			cmd->writeFd = cmd->next->outPipe.in_fd;
		}
	}
}

void showCmd(int fd, Cmd *cmd) {
	int i;
	for (i = 0; i < cmd->argc; i++) {
		dprintf(fd, "argv[%d]=%s\n", i, cmd->argv[i]);
	}

}
static void clearSignal() {
	signal(SIGQUIT, SIG_DFL);
	signal(SIGKILL, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE, SIG_DFL);
}

int spawn(Client *client, Cmd *cmd) {
	char *prog = cmd->argv[0];
	char **arg_list = cmd->argv;
	pid_t child = 0;

	//dup2Cmd(cmd);
	setCmd(client, cmd);
	if (cmd->wait != 0) {
		client->waitPid = fork();
		child = client->waitPid;
	} else {
		child = fork();
	}

	if (child != 0) {
		if (child > 0) {
			dprintf(stdo_fd, "(%d)%s \n", getpid(), cmd->argv[0]);
			cmd->p_id = child;
		}
		return child;
	} else {
		clearSignal();
		//close(0);
		//close(1);
		//dup(cmd->readFd);
		//dup(cmd->writeFd);
		dup2(cmd->readFd, 0);
		dup2(cmd->writeFd, 1);
		if (client->has_err_pipe == -1) {
			//close(2);
			//dup(1);
			//dup(console.out_fd);
			dup2(client->conSocketFd, 2);
		} else {
			//close(2);
			//dup(erro_fd);
			dup2(cmd->writeFd, 2);
		}

#ifdef debug

		dprintf(stdo_fd, "(%d)%s ", getpid(), cmd->argv[0]);
		if (!isNull(&cmd->outPipe))
			dprintf(stdo_fd, "i(%d) o(%d) ", cmd->outPipe.in_fd,
					cmd->outPipe.out_fd);
		int fo = fcntl(cmd->readFd, F_GETFL, 0);
		int fi = fcntl(cmd->writeFd, F_GETFL, 0);
		dprintf(stdo_fd, "%d-%d(%d) %d-%d(%d)\n", cmd->readFd, 0, fo,
				cmd->writeFd, 1, fi);
#endif

		int i = 0;
		for (i = FD_SETSIZE; i > 2; i--) {

#ifdef debug
			if (i != stdo_fd)
#endif
				close(i);
		}
		setenv("PATH", client->PATH, 1);
		int re_exec = execvp(prog, arg_list);

#ifdef debug
		dprintf(stdo_fd, "exec(%d)%s errno=(%d)%s ", strlen(prog), prog, errno,
				strerror(errno));
		//showCmd(cmd);
#endif
		fprintf(stderr, "Unknown command: [%s].\n", prog);
		close(0);
		close(1);
		close(2);
		exit(-1);
	}
	return child;
}

int indexOf(char *str1, char *str2) {
	char *p = str1;
	int i = 0;
	p = strstr(str1, str2);
	if (p == NULL)
		return -1;
	else {
		while (str1 != p) {
			str1++;
			i++;
		}
	}
	return i;
}

void waitOver(int pid) {
	int status;
	do {
		//if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1)
		if (kill(pid, 0) < 0)
			break;
		else
			usleep(1000000);
	} while (1);
}

void showMotd(Client *client) {
	char *remote_ip = client->remote_ip;
	int fd = client->conSocketFd;
	dprintf(stdo_fd, "(%d)%s open\n", getpid(), remote_ip);
	dprintf(fd, "****************************************\n");
	dprintf(fd, "** Welcome to the information server. **\n");
	dprintf(fd, "****************************************\n");
	//       printf("Your From %s:%d\n",remote_ip,remote_port);
}
/*
 static void closedClient(Client *client,int signo) {
 kill(getpid(), SIGKILL);
 clearSignal();
 fflush(stdin);
 fflush(stdout);
 write(client->conSocketFd, '\0', 1);
 shutdown(client->conSocketFd, SHUT_RDWR);


 close(client->clientSendPipe[1]);
 close(client->mainSendPipe[0]);
 close(client->conSocketFd);
 close(console.out_fd);
 close(console.in_fd);
 close(0);
 close(1);
 close(2);
 close(stdo_fd);

 closeAllFd();
 exit(EXIT_SUCCESS);
 }
 */
static void closeClient(Client *c) {
	char msg[255];
	shutdown(c->conSocketFd, 2);
	c->pid = 0;
	memset(c, 0, sizeof(c));
	dprintf(stdo_fd, "%s client exit", c->name);
	sprintf(msg, "*** User '%s' left. ***\n", c->name);
	clientCastMsg(msg, -1);
}
int readSocket(Client *c) {
	int len;
	if ((len = read(c->conSocketFd, c->read_buffer_w, sizeof(c->read_buffer)
			- 1)) < 0)
		return -1;
	c->read_buffer_w += len;
	return len;
}

int readLine(Client *c) {
	memset(c->cmd_buffer, 0, sizeof(c->cmd_buffer));

	sscanf(c->read_buffer_r, "%[^\n]", c->cmd_buffer);

	char *final = c->read_buffer_r + strlen(c->cmd_buffer);
#ifdef debug
	//	dprintf(1,"readLine(%p)%p\n",c->read_buffer_w,final);
#endif
	if (*final != '\n') {
		memset(c->cmd_buffer, 0, sizeof(c->cmd_buffer));
	} else {

		char *final_c = c->cmd_buffer + strlen(c->cmd_buffer);
		*final_c = '\n';
		c->read_buffer_r += strlen(c->cmd_buffer);
#ifdef debug
		//		dprintf(1,"readLine(%p)%p\n",c->read_buffer_w,c->read_buffer_r);
#endif
		if (c->read_buffer_r == c->read_buffer_w) {
			memset(c->read_buffer, 0, sizeof(c->read_buffer));
			c->read_buffer_r = c->read_buffer;
			c->read_buffer_w = c->read_buffer;

		}
	}

	return strlen(c->cmd_buffer);
}
static void nextQueCmd(Client *c) {
	int qi = c->qi;
	closePipe(&c->queFd[qi]);
	c->qi = c->qi + 1;
	c->qi = c->qi % queMAX;
}
static void readClientCmd(int clientId, fd_set *rfds_src) {
	Client *client = &clientQueue[clientId];

	int len = readSocket(client);
#ifdef debug
	/////////////		dprintf(stdo_fd, "readClientCmd(%d)\n",len);
#endif
	if (len <= 0) {
		FD_CLR(client->conSocketFd ,rfds_src);
		closeClient(client);
		return;
	}

	len = readLine(client);
	memset(client->cmd_src, 0, sizeof(client->cmd_src));
	sscanf(client->cmd_buffer, "%[^\r\n]", client->cmd_src);
	char *cmd_src = client->cmd_src;
	int i;
	char tmp[255], msg[255];
	memset(msg, 0, sizeof(msg));
	memset(tmp, 0, sizeof(tmp));
	char *req;
	req = strtok(client->cmd_buffer, "\r\n/");

#ifdef debug
	//dprintf(stdo_fd, "(%d+%d)%%%s\n", getpid(), qi, req);
#endif
	if (!req) {
		nextQueCmd(client);
		return;
	}
	if (strcmp(req, "printenv PATH") == 0) {
		dprintf(client->conSocketFd, "PATH=%s\n", client->PATH);
		nextQueCmd(client);
		return;
	} else if (strncmp(req, "setenv PATH", 11) == 0) {
		memset(client->PATH, 0, sizeof(client->PATH));
		sscanf(req, "setenv PATH %s", client->PATH);

		nextQueCmd(client);
		return;
	} else if (strncmp(req, "exit", 4) == 0) {
		closeClient(client);
		nextQueCmd(client);
		return;
	}

	else if (strncmp(req, "who", 3) == 0) {
		whoCmd(client->conSocketFd, clientId);
		nextQueCmd(client);
		return;
	}

	else if (strncmp(req, "name ", 5) == 0) {
		sprintf(client->name, "%s", &req[5]);
		sprintf(msg, "*** User from %s/%d is named '%s'. ***\n",
				client->remote_ip, client->remote_port, client->name);
		clientCastMsg(msg, -1);
		dprintf(stdo_fd, "%s", msg);

		nextQueCmd(client);
		return;
	}

	else if (strncmp(req, "yell ", 5) == 0) {
		sprintf(msg, "*** %s yelled ***: %s\n", client->name, &req[5]);
		clientCastMsg(msg, -1);
		dprintf(stdo_fd, "%s", msg);
		nextQueCmd(client);
		return;
	} else if (strncmp(req, "tell ", 5) == 0) {

		int i = atoi(&req[5]);
		if (i > 0 && i <= ClientQueueLength) {
			char pmsg[255];
			memset(pmsg, 0, 255);
			sscanf(req, "tell %d %[^\n]", &i, pmsg);
			int index = i - 1;
			if (clientQueue[index].pid > 0) {
				sprintf(msg, "*** %s told you ***: %s\n", client->name, pmsg);
				dprintf(clientQueue[index].conSocketFd, "%s", msg);
			} else {
				dprintf(client->conSocketFd,
						"*** Error: user #%d does not exist yet. ***\n", i);

			}
		} else {
			dprintf(client->conSocketFd,
					"*** Error: user #%d does not exist yet. ***\n", i);

		}

		nextQueCmd(client);
		//write(client->clientSendPipe[1], req, strlen(req));
		//closePipe(&queFd[qi]);
		//*qi++;
		//*qi %= queMAX;
		////fflush(stdout);
		//FD_CLR(0,rfds_src);
		return;
	}

	client->has_err_pipe = indexOf(req, "!");
	client->has_fifoOut = indexOf(req, ">|");
	if (client->has_fifoOut > 0) {
		req[(client->has_fifoOut) + 1] = ' ';
	} else {
		client->has_fifoOut = indexOf(req, ">!");
		if (client->has_fifoOut > 0) {
			req[(client->has_fifoOut) + 1] = ' ';
			client->has_err_pipe = -1;
		}
	}

	client->has_fifoIn = indexOf(req, "<");

	client->fifoInNumber = -1;
	if (client->has_fifoIn > 0) {
		if (req[(client->has_fifoIn) + 1] == ' ') {
			client->has_fifoIn = -1;
		} else {
			client->fifoInNumber = atoi(&req[client->has_fifoIn + 1]);
		}
	}

	Array *arr = split(req, "|!");

	clientQueue[clientId].cmd.argc
			= ToArray(arr, &clientQueue[clientId].cmd.argv);
	Cmd *subCmd = malloc(sizeof(Cmd) * client->cmd.argc);
	Cmd *nextCmd = subCmd;
	Cmd *prevCmd = NULL;
	client->cmd.first = NULL;

	int qi = client->qi;
	int delay = 0;
	PipeFD *queFd = &client->queFd[0];
	for (i = 0; i < client->cmd.argc; i++) {

		*nextCmd = getCmd(client->cmd.argv[i], " \t");

		//showCmd(stdo_fd,nextCmd);
		if (nextCmd->argc == 0)
			continue;

		if (client->cmd.first == NULL) {

			client->erro_fd = -1;
			client->cmd.first = nextCmd;
			client->cmd.next = nextCmd;
			if (!isNull(queFd + qi)) {
				//nextCmd->outPipe = queFd[qi];
				nextCmd->readFd = queFd[qi].out_fd;

				//if(erro_fd==queFd[qi].in_fd)
			}

		}

		if (i == client->cmd.argc - 1 && i != 0) {//final
			delay = atoi(nextCmd->argv[0]);
			if (delay > 0)
				break;
		}

		if (prevCmd) {
			prevCmd->next = nextCmd;
			nextCmd->prev = prevCmd;
		}
		nextCmd->parent = &client->cmd;
		nextCmd->first = client->cmd.first;
		prevCmd = nextCmd;
		nextCmd++;
	}

	int dstqi = (qi + delay) % queMAX;

#ifdef debug
	if (client->has_err_pipe != -1)
		dprintf(stdo_fd, "(%d->%d)has error pipe %d\n", getpid(), delay,
				client->has_err_pipe);
#endif

	if (delay > 0) {

		nextCmd = prevCmd;
		if (isNull(queFd + dstqi))
			queFd[dstqi] = newPipeFD();
		nextCmd->writeFd = queFd[dstqi].in_fd;

		//if (has_err_pipe > 0)
		//	erro_fd = queFd[dstqi].in_fd;

#ifdef debug
		dprintf(stdo_fd, "(%d->%d)+%d", getpid(), delay, dstqi);
		dprintf(stdo_fd, "i(%d) o(%d)\n", queFd[dstqi].in_fd,
				queFd[dstqi].out_fd);
#endif
	}

	int child_pid = 0;
	nextCmd = client->cmd.first;
	while (nextCmd) {
		child_pid = spawn(client, nextCmd);
		if (nextCmd == client->cmd.first&&!isNull(queFd + qi)) {
			close(queFd[qi].in_fd);
		}

		if (child_pid == 0) //child exec error
		{
			exit(EXIT_SUCCESS);
		} else if (child_pid < 0) {//fork error
			printf("Shell Error may fork too much.\n");
			break;
			//continue;
		}
		PipeFD *nextquefd = queFd + ((qi + 1) % queMAX);
		PipeFD *dstquefd = queFd + dstqi;
		if (nextCmd->writeFd != client->conSocketFd) {
			if (dstquefd->in_fd != nextCmd->writeFd) {
				if (nextCmd->writeFd > 2) {
					close(nextCmd->writeFd);
				}
			}
			/*else {
				if (nextquefd->in_fd == nextCmd->writeFd) {
						if (nextCmd->writeFd > 2) {
							close(nextCmd->writeFd);
						}
				}

			}*/
		}

		if (nextCmd->readFd != client->conSocketFd) {
			if (nextCmd->readFd > 2) {
				close(nextCmd->readFd);
			}
		}

		/*
		 if (nextCmd->outPipe.in_fd != client->conSocketFd) {

		 if (isNull(nextquefd)) {
		 closeInput(&nextCmd->outPipe);
		 } else {
		 if (nextCmd->outPipe.in_fd != nextquefd->in_fd)
		 closeInput(&nextCmd->outPipe);
		 }
		 }
		 */

		//if (delay == 0 || nextCmd->next != NULL)
			waitOver(nextCmd->p_id);

		/*
		 if (nextCmd->outPipe.out_fd != client->conSocketFd) {
		 if (isNull(dstquefd)) {
		 closeOutput(&nextCmd->outPipe);
		 } else {
		 if (nextCmd->outPipe.out_fd != dstquefd->out_fd)
		 closeOutput(&nextCmd->outPipe);
		 }
		 }
		 */

		nextCmd = nextCmd->next;
	}



	nextQueCmd(client);
	//free(subCmd);
}

/*
 static void clientInit(Client *client,int listenSocketFd) {
 int *qi=client->qi;
 PipeFD client->consloe=client->console;
 clearSignal();
 //signal(SIGINT, closedClient);
 //signal(SIGKILL, closedClient);
 //signal(SIGTERM, closedClient);
 //signal(SIGQUIT, closedClient);
 //signal(SIGCHLD, handlerChld);
 //close(client->clientSendPipe[0]);
 //close(client->mainSendPipe[1]);
 //close(0);
 //close(1);
 int flags = fcntl(client->conSocketFd, F_GETFL, 0);
 fcntl(client->conSocketFd, F_SETFL, flags | O_NONBLOCK);
 //dup(client->conSocketFd);
 //dup(client->conSocketFd);
 //erro_fd = -1;
 //close(listenSocketFd);
 //close(stdi_fd);
 memset(client->queFd, -1, queMAX * sizeof(PipeFD));
 client->qi = 0;
 showMotd(client);
 console.out_fd = dup(client->conSocketFd);
 console.in_fd = dup(client->conSocketFd);
 client->cmd = (Cmd*) malloc(sizeof(Cmd));
 client->cmd->p_id = getpid();
 client->cmd->outPipe = console;
 }
 */

static int allClientAction(int(*action)(Client *)) {
	int i = 0;
	Client *c;
	for (i = 0; i < ClientQueueLength; i++) {
		c = &clientQueue[i];
		if (action(c) == -1) {
			return i;
		}
	}
	return i;
}
/*
 static int acceptClient(int listenSocketFd) {
 int socksize = sizeof(struct sockaddr_in);
 int conSocketFd = accept(listenSocketFd, (struct sockaddr *) &dest,
 (socklen_t*) &socksize);
 if (conSocketFd >= 0) {
 int clientIndex = clietnQueueFlowId;
 int i = 0;
 while (clientQueue[clientIndex].pid > 0) {
 clietnQueueFlowId++;
 clietnQueueFlowId %= ClientQueueLength;
 clientIndex = clietnQueueFlowId;
 i++;
 if (i >= ClientQueueLength) {
 clientIndex = ClientQueueLength;
 break;
 }
 }
 clietnQueueFlowId = 0;
 dprintf(stdo_fd, "%d.\n", clientIndex);
 if (clientIndex >= ClientQueueLength) {
 dprintf(conSocketFd, "server is person limit.\n");
 close(conSocketFd);
 return -1;
 }
 client = &clientQueue[clientIndex];

 dprintf(stdo_fd, "%d+ accept ok\n", clientIndex);
 beginUpdateClientQueue();
 getClientQueue();
 client->conSocketFd = conSocketFd;
 client->index = clientIndex;
 strcpy(client->name, ClientDefaultName);
 strcpy(client->remote_ip, inet_ntoa(dest.sin_addr));
 client->remote_port = dest.sin_port;
 client->fifo_fd[0] = -1;
 client->fifo_fd[1] = -1;
 sprintf(client->fifo_path, "/tmp/proj2fifo%d", clientIndex);
 pipe(client->mainSendPipe);
 pipe(client->clientSendPipe);
 endUpdateClientQueue();
 //client->
 client->pid = fork();
 if (client->pid > 0) {//root
 int i = client->pid;
 beginUpdateClientQueue();
 getClientQueue();
 client->pid = i;
 endUpdateClientQueue();
 close(client->mainSendPipe[0]);
 close(client->clientSendPipe[1]);
 close(client->conSocketFd);
 client = NULL;

 return clientIndex;
 } else {
 clientHandler(listenSocketFd);
 exit(EXIT_SUCCESS);
 }
 }
 return -1;
 }
 */
static int acceptClient(int listenSocketFd) {
	int socksize = sizeof(struct sockaddr_in);
	struct sockaddr_in dest;
	int conSocketFd = accept(listenSocketFd, (struct sockaddr *) &dest,
			(socklen_t*) &socksize);
	if (conSocketFd >= 0) {
		int clientIndex = clietnQueueFlowId;
		int i = 0;
		while (clientQueue[clientIndex].pid > 0) {
			clietnQueueFlowId++;
			clietnQueueFlowId %= ClientQueueLength;
			clientIndex = clietnQueueFlowId;
			i++;
			if (i >= ClientQueueLength) {
				clientIndex = ClientQueueLength;
				break;
			}
		}
		clietnQueueFlowId = 0;
		dprintf(stdo_fd, "%d.\n", clientIndex);
		if (clientIndex >= ClientQueueLength) {
			dprintf(conSocketFd, "server is person limit.\n");
			close(conSocketFd);
			return -1;
		}
		Client *client = &clientQueue[clientIndex];
		client->conSocketFd = conSocketFd;
		client->index = clientIndex;
		strcpy(client->name, ClientDefaultName);
		strcpy(client->remote_ip, inet_ntoa(dest.sin_addr));
		memset(client->cmd_buffer, 0, sizeof(client->cmd_buffer));
		memset(client->read_buffer, 0, sizeof(client->read_buffer));
		client->read_buffer_r = client->read_buffer;
		client->read_buffer_w = client->read_buffer;
		client->remote_port = ntohs(dest.sin_port);
		client->fifo_fd[0] = -1;
		client->fifo_fd[1] = -1;
		client->erro_fd = -1;
		client->listenCmd = 0;
		sprintf(client->PATH, "bin:.");
		sprintf(client->fifo_path, "/tmp/proj2fifo%d", clientIndex);
		client->pid = getpid();
		showMotd(client);
		char msg[255];
		sprintf(msg, "*** User '%s' entered from %s/%d. ***\n", client->name,
				client->remote_ip, client->remote_port);
		clientCastMsg(msg, -1);
		return clientIndex;
	}
	return -1;
}
int listenSocketFd;

static void readCmd() {
	char line[255];
	fgets(line, 255, stdin);
	if (strcmp(line, "who\n") == 0) {
		whoCmd(stdo_fd, -1);
	} else if (strcmp(line, "kill\n") == 0) {
		//	killClient(&clientQueue[0]);
	}
	//printf("sss:%s",input);
}
/*
 static void closedMain() {
 allClientAction(killClient);
 close(listenSocketFd);
 exit(EXIT_SUCCESS);

 }
 */
static void closedMain() {
	shutdown(listenSocketFd, 2);
	//allClientAction(killClient);
	closeAllFd();
	exit(EXIT_SUCCESS);
}
void mainClientSignHandler(int signo) {

	//int oldmask = sigblock(sigmask(SIGUSR1));
	if (signo == SIGUSR1) {
		dprintf(stdo_fd, "user1\n");
	}
	//sigsetmask(oldmask);
	if (signo == SIGPIPE) {
		dprintf(stdo_fd, "pipe broke%d\n", getpid());
	}
	if (signo == SIGCHLD) {
		dprintf(stdo_fd, "kill%d \n", getpid());
		//kill(getpid(), SIGUSR1);
	}

}

/*
 static void checkAllClientRpipe(fd_set* rfds_src, fd_set* rfds) {
 int i;
 char msg[255];

 Client *c;
 for (i = 0; i < ClientQueueLength; i++) {
 c = &clientQueue[i];
 if (c->pid > 0) {
 if (FD_ISSET(c->clientSendPipe[0],rfds)) {
 char tmp[1024];
 int len = read(c->clientSendPipe[0], tmp, 1024);
 if (len > 0) {
 tmp[len] = '\0';
 if(strncmp("who",tmp,3)==0){
 whoCmd(c->mainSendPipe[1]);
 dprintf(stdo_fd, "%s", tmp);
 }else


 if (strncmp("name ", tmp, 5) == 0) {
 //sscanf(tmp,"name %s\n",c->name);
 sprintf(c->name, "%s", &tmp[5]);

 sprintf(msg,
 "*** User  from %s/%d is named '%s'. ***\r\n",
 c->remote_ip, c->remote_port, c->name);
 clientCastMsg(msg, -1);
 dprintf(stdo_fd, "%s", msg);
 } else if (strncmp("yell ", tmp, 5) == 0) {
 sprintf(msg, "*** %s yelled ***:%s\r\n", c->name,
 &tmp[5]);
 clientCastMsg(msg, -1);
 dprintf(stdo_fd, "%s", msg);
 } else if (strncmp("tell ", tmp, 5) == 0) {
 tmp[len] = '\n';
 int i = atoi(&tmp[5]);
 if (i > 0 && i <= ClientQueueLength) {
 char pmsg[255];
 memset(pmsg, 0, 255);
 sscanf(tmp, "tell %d %[^\n]", &i, pmsg);
 int index = i - 1;
 if (clientQueue[index].pid > 0) {
 sprintf(msg, "*** %s told you ***:%s\r\n",
 c->name, pmsg);
 dprintf(clientQueue[index].mainSendPipe[1],
 "%s", msg);
 //dprintf(stdo_fd, "%s", tmp);
 } else {
 dprintf(
 c->mainSendPipe[1],
 "*** Error: user #%d does not exist yet. ***\r\n",
 i);
 }

 } else {
 dprintf(
 c->mainSendPipe[1],
 "*** Error: user #%d does not exist yet. ***\r\n",
 i);
 }
 } else if (strncmp("fifo_w", tmp, 6) == 0) {
 char ccmd[255];
 sscanf(tmp, "fifo_w %[^\n]", ccmd);
 sprintf(
 msg,
 "*** %s (#%d) just piped '%s' into his/her pipe. ***\r\n",
 c->name, c->index + 1, ccmd);
 clientCastMsg(msg, -1);
 dprintf(stdo_fd, "%s", msg);
 } else if (strncmp("fifo_r", tmp, 6) == 0) {
 char ccmd[255];
 int revice_i;
 sscanf(tmp, "fifo_r %d %[^\n]", &revice_i, ccmd);
 Client *c2 = &clientQueue[revice_i];
 sprintf(
 msg,
 "*** %s (#%d) just received the pipe from %s (#%d) by '%s' ***\n",
 c->name, c->index + 1, c2->name, revice_i + 1,
 ccmd);
 clientCastMsg(msg, -1);
 dprintf(stdo_fd, "%s", msg);
 }

 } else {
 dprintf(stdo_fd, "%s client exit", c->name);
 sprintf(msg, "*** User '%s' left. ***\r\n", c->name);
 clientCastMsg(msg, i);
 FD_CLR(c->clientSendPipe[0],rfds_src);
 killClient(c);
 }
 }
 }
 }
 }
 */

int recv_msg(int inFD, int outFD) {
	char buf[3000], *tmp;
	int len;
	if ((len = read(inFD, buf, sizeof(buf) - 1)) < 0)
		return -1;

	buf[len] = 0;
	if (len > 0) {
		for (tmp = strtok(buf, "\n"); tmp; tmp = strtok(NULL, "\n")) {
			//dprintf(outFD,"%s\n",tmp);      //echo input
		}
	}

	return len;
}

static void listenClientCmd(Client *c, fd_set *rfds_src) {
	if (c->listenCmd == 0) {
		c->listenCmd = 1;
		dprintf(c->conSocketFd, "%% ");
		FD_SET(c->conSocketFd,rfds_src);
	}

}
static void handerClientInput(fd_set* rfds_src, fd_set* rfds) {
	int i;
	Client *c;
	for (i = 0; i < ClientQueueLength; i++) {
		c = &clientQueue[i];
		if (c->pid > 0) {
			if (FD_ISSET(c->conSocketFd,rfds)) {
				FD_CLR(c->conSocketFd,rfds);
				c->listenCmd = 0;
				readClientCmd(i, rfds_src);
				listenClientCmd(c, rfds_src);

			}

		}

	}
}
static void handlerChld(int signo) {

	if (signo == SIGCHLD) {
		int status;
		int chld_pid = waitpid(-1, &status, WUNTRACED | WCONTINUED);
		int i = 0;
		for (i = 0; i < ClientQueueLength; i++) {
			if (clientQueue[i].waitPid == chld_pid) {
				clientQueue[i].waitPid = 0;
				listenClientCmd(&clientQueue[i], &main_rfds_src);
				dprintf(stdo_fd, "child kill %d %d", chld_pid,
						clientQueue[i].conSocketFd);
			}

		}
	}

}
int main(int argc, char *argv[], char *envp[]) {
	printf("Pipe buffer size=%d\n", PIPE_BUF);
	printf("FD size=%d\n", __FD_SETSIZE);

	chdir("ras");
	char tmp[255];
	getcwd(tmp, 255);
	setenv("PWD", tmp, 1);
	setenv("PATH", "bin:.", 1);
	printf("PWD=%s\n", getenv("PWD"));
	printf("PATH=%s\n", getenv("PATH"));

	InitClientQueue();

	printf("Input Server Port:");
	char input[10];
	fgets(input, 9, stdin);
	if (atoi(input) != 0)
		PORTNUM = atoi(input);

	stdi_fd = dup(0);
	stdo_fd = dup(1);

	struct sockaddr_in serv;
	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(PORTNUM);

	listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);

	bind(listenSocketFd, (struct sockaddr *) &serv, sizeof(struct sockaddr));

	listen(listenSocketFd, 5);
	signal(SIGKILL, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGKILL, closedMain);
	signal(SIGINT, closedMain);
	signal(SIGTERM, closedMain);
	signal(SIGQUIT, closedMain);
	int flags = fcntl(listenSocketFd, F_GETFL, 0);
	fcntl(listenSocketFd, F_SETFL, flags | O_NONBLOCK);

	signal(SIGUSR1, mainClientSignHandler);
	signal(SIGCHLD, handlerChld);
	signal(SIGPIPE, mainClientSignHandler);

	fd_set rfds;
	FD_ZERO(&main_rfds_src);
	FD_SET(0,&main_rfds_src);
	FD_SET(listenSocketFd,&main_rfds_src);
	dprintf(stdo_fd, "# ");
	while (1) {
		bcopy((const void*) &main_rfds_src, (void*) &rfds, sizeof(fd_set));
		int checkLength = __FD_SETSIZE;
		int s = select(checkLength, &rfds, (fd_set*) 0, (fd_set*) 0,
				(struct timeval*) 0);
		if (s == -1) {
			dprintf(stdo_fd, "select -1\n");
			continue;
		}

		if (FD_ISSET(listenSocketFd,&rfds)) {
			int index = acceptClient(listenSocketFd);
			Client *c = &clientQueue[index];
			char msg[255];
			sprintf(msg, "*** User '%s' entered from %s/%d. ***\n", c->name,
					c->remote_ip, c->remote_port);
			//clientCastMsg(msg, -1);
			listenClientCmd(c, &main_rfds_src);
		}

		if (FD_ISSET(0,&rfds)) {
			readCmd();
			dprintf(stdo_fd, "# ");
		}
		handerClientInput(&main_rfds_src, &rfds);

	}

	dprintf(stdo_fd, "server stop\n");
	shutdown(listenSocketFd, 2);
	exit(EXIT_SUCCESS);
}

