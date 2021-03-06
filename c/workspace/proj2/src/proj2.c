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
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>
#include "Array.h"
#include "ClientQueue.h"

#define SEMKEY 222L
#define SHMKEY 222
#define PERMS 0666
static Client *client = NULL;
void fifoPipe(int * fd, char *fifo_path) {
	int status = mkfifo(fifo_path, 0666);
	int tmp_out = open(fifo_path, O_RDONLY | O_NONBLOCK);
	int tmp_in = open(fifo_path, O_WRONLY);
	*fd = tmp_out;
	fd += 1;
	*fd = tmp_in;
}

static struct sembuf op_lock[2] = { { 0, 0, 0 }, { 0, 1, SEM_UNDO } };
static struct sembuf op_unlock[1] = { { 0, -1, (IPC_NOWAIT | SEM_UNDO) } };

int sem_id = -1;
int fifo_sem_id = -1;
void lock() {

	if (sem_id < 0) {
		sem_id = semget(SEMKEY, 1, IPC_CREAT | PERMS);
	}
	semop(sem_id, &op_lock[0], 2);
}

void unlock() {
	semop(sem_id, &op_unlock[0], 1);
}



static void setClientQueue() {
	int shmid = shmget((key_t) SHMKEY, (size_t) sizeof(Client)
			* ClientQueueLength, 0666 | IPC_CREAT);
	Client* shmem = (Client*) shmat(shmid, (Client *) 0, 0);
	memcpy(shmem, &clientQueue, (size_t) sizeof(Client) * ClientQueueLength);
}
static void getClientQueue() {
	int shmid = shmget((key_t) SHMKEY, (size_t) sizeof(Client)
			* ClientQueueLength, 0666 | IPC_CREAT);
	Client* shmem = (Client*) shmat(shmid, (Client *) 0, 0);
	memcpy(&clientQueue, shmem, (size_t) sizeof(Client) * ClientQueueLength);
}
static void beginUpdateClientQueue() {
	lock();
}
static void endUpdateClientQueue() {
	setClientQueue();
	unlock();
}
static void whoCmd(int writeFd) {
	lock();
	getClientQueue();
	unlock();
	int i;
	int pid = getpid();
	dprintf(writeFd, "<ID>\t<nickname>\t<IP/port>\t<indicate me>\n");
	for (i = 0; i < ClientQueueLength; i++) {
		Client *c = &clientQueue[i];
		char isme[] = "<-me";
		if (c->pid > 0) {
			if (c->pid == pid) {
				dprintf(writeFd, "%d\t%s\t%s/%d\t%s\n", c->index + 1,
						c->name, c->remote_ip, c->remote_port, isme);
			} else {
				dprintf(writeFd, "%d\t%s\t%s/%d\t\n", c->index + 1, c->name,
						c->remote_ip, c->remote_port);
			}
		}
	}
}

#define ndebug

#define queMAX 32767
int PORTNUM = 8000;

int erro_fd;
int stdo_fd;
int stdi_fd;
fd_set client_rfds_src;
static int mainPid = -1;
static char cmd_src[255];

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

typedef struct PipeFD {
	int out_fd;
	int in_fd;
} PipeFD;

PipeFD newPipeFD() {
	int fd[2];
	pipe(fd);
	PipeFD pfd = { fd[0], fd[1] };
	return pfd;
}
void closePipe(PipeFD *fd) {
	close(fd->in_fd);
	close(fd->out_fd);
	fd->in_fd = -1;
	fd->out_fd = -1;
}
void closeOutput(PipeFD *fd) {
#ifdef debug
	dprintf(stdo_fd, "(%d)closeOuput(%d)  ", getpid(), fd->out_fd);
#endif
	close(fd->out_fd);
}
void closeInput(PipeFD *fd) {
#ifdef debug
	dprintf(stdo_fd, "(%d)closeInput(%d)  ", getpid(), fd->in_fd);
#endif
	close(fd->in_fd);
}
int isNull(PipeFD *fd) {
	return (fd->in_fd == -1 && fd->out_fd == -1);
}
typedef struct Cmd {
	int p_id;
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
int has_err_pipe = -1;
int has_fifoOut = -1;
int has_fifoIn = -1;
int fifoInNumber = -1;

void setCmd(Cmd *cmd) {
	int i, fd = 1;

	for (i = 1; i < cmd->argc; i++) {
		if (strncmp(cmd->argv[i], ">", 1) == 0) {

			if (has_fifoOut > 0) {
                                 lock();
                                 getClientQueue();
                                 unlock();
				//dprintf(1,"user exist\n");
				if (client ->fifo_fd[1] == -1) {

					has_err_pipe = indexOf(cmd_src, ">!");

					beginUpdateClientQueue();
					getClientQueue();
					//fifoPipe(client->fifo_fd, client->fifo_path);
					
				        client->fifo_fd[1] = open(client->fifo_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR
							| S_IWUSR | S_IRGRP | S_IWGRP);
					endUpdateClientQueue();
					cmd->writeFd = client->fifo_fd[1];
					char fifocmd[255];
					sprintf(fifocmd,"fifo_w %s\n",cmd_src);
					write(client->clientSendPipe[1], fifocmd, strlen(fifocmd));
					FD_CLR(0,&client_rfds_src);
				} else {
					dprintf(1, "*** Error: your pipe already exists. ***\n");
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

					lock();
					getClientQueue();
					unlock();
					Client *c = &clientQueue[clientIndex];
					if (c->pid > 0) {
						//if (c->fifo_fd[0] > 0) {
						if (c->fifo_fd[1] > 0) {
							beginUpdateClientQueue();
							getClientQueue();
							int fd[2];
							//fifoPipe((int *)&fd, c->fifo_path);

				                        fd[0] = open(c->fifo_path, O_RDONLY, S_IRUSR | S_IWUSR | S_IRGRP
						               | S_IWGRP);
							c->fifo_fd[1] = -1;
							endUpdateClientQueue();
							cmd->readFd = fd[0];
							char fifocmd[255];
							sprintf(fifocmd,"fifo_r %d %s\n",clientIndex,cmd_src);
							write(client->clientSendPipe[1], fifocmd, strlen(fifocmd));
							FD_CLR(0,&client_rfds_src);
							//close(fd[1]);
						} else {
							dprintf(
									1,
									"*** Error: the pipe from #%d does not exist yet. ***\n",
									fifoInNumber);
							cmd->readFd = getNullFd();
					        cmd->writeFd = getNullFd();


						}
					} else {
						dprintf(
								1,
								"*** Error: the pipe from #%d does not exist yet. ***\n",
								fifoInNumber);
						cmd->readFd = getNullFd();
					        cmd->writeFd = getNullFd();
					}

				}

				else {
					dprintf(
							1,
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
			cmd->readFd = cmd->parent->outPipe.out_fd;

		} else {
			cmd->readFd = cmd->outPipe.out_fd;
		}
	}

	if (cmd->writeFd == -1) {
		if (cmd->next == NULL)
			cmd->writeFd = cmd->parent->outPipe.in_fd;
		else {
			if (isNull(&cmd->next->outPipe))
				cmd->next->outPipe = newPipeFD();
			cmd->writeFd = cmd->next->outPipe.in_fd;
		}
	}
}

PipeFD console;
void showCmd(Cmd *cmd) {
	int i;
	for (i = 0; i < cmd->argc; i++) {
		printf("argv[%d]=%s\n", i, cmd->argv[i]);
	}

}
static void clearSignal() {
	signal(SIGQUIT, SIG_DFL);
	signal(SIGKILL, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGUSR1, SIG_DFL);
	signal(SIGCHLD, SIG_DFL);
	signal(SIGPIPE,SIG_DFL);
}
int spawn(Cmd *cmd) {
	char *prog = cmd->argv[0];
	char **arg_list = cmd->argv;

	pid_t child = 0;

	//dup2Cmd(cmd);
	setCmd(cmd);
	child = fork();

	if (child != 0) {
		if (child > 0) {
			cmd->p_id = child;
		}
		return child;
	} else {
		//clearSignal();
		//close(0);
		//close(1);
		//dup(cmd->readFd);
		//dup(cmd->writeFd);
		dup2(cmd->readFd, 0);
		dup2(cmd->writeFd, 1);
		if (erro_fd == -1) {
			//close(2);
			//dup(1);
			//dup(console.out_fd);
			dup2(console.out_fd, 2);
		} else {
			//close(2);
			//dup(erro_fd);
			dup2(erro_fd, 2);
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
		for (i = 1024; i > 2; i--) {

#ifdef debug
			if (i != stdo_fd)
#endif
			close(i);
		}

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
		if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1)
			break;
	} while (!
	WIFEXITED(status) && !WIFSIGNALED(status));
}

struct sockaddr_in dest;
struct sockaddr_in serv;

void showMotd() {
	char *remote_ip = client->remote_ip;
	dprintf(stdo_fd, "(%d)%s open\n", getpid(), remote_ip);
	printf("****************************************\n");
	printf("** Welcome to the information server. **\n");
	printf("****************************************\n");
	//       printf("Your From %s:%d\n",remote_ip,remote_port);
}

static void closedClient(int signo) {
        kill(getpid(),SIGKILL);
	clearSignal();
	fflush(stdin);
	fflush(stdout);
	write(client->clientSendPipe[1], "close\n", 7);
	write(client->conSocketFd, '\0', 1);
	shutdown(client->conSocketFd, SHUT_RDWR);

	/*
	 close(client->clientSendPipe[1]);
	 close(client->mainSendPipe[0]);
	 close(client->conSocketFd);
	 close(console.out_fd);
	 close(console.in_fd);
	 close(0);
	 close(1);
	 close(2);
	 close(stdo_fd);
	 */
	closeAllFd();
	exit(EXIT_SUCCESS);
}

PipeFD queFd[queMAX];
;
Cmd *clientCmd;
int qi;
static void readClientCmd(fd_set *rfds_src) {

	char buffer[255];
	char tmp[255];
	char *req;
	int i;
	memset(buffer, 0, 255);
	fflush(stdout);
	fflush(stdin);
	if (!fgets(buffer, 255, stdin)) {
		closedClient(SIGKILL);
	}
	sscanf(buffer,"%[^\r\n]",cmd_src);
	req = strtok(buffer, "\r\n/");

#ifdef debug
	dprintf(stdo_fd, "(%d+%d)%%%s\n", getpid(), qi, req);
#endif
	if (!req) {
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		fflush(stdout);
		return;
	}
	if (strcmp(req, "printenv PATH") == 0) {
		printf("PATH=%s\n", getenv("PATH"));
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		fflush(stdout);
		return;
	} else if (strncmp(req, "setenv PATH", 11) == 0) {
		sscanf(req, "setenv PATH %s", tmp);
		setenv("PATH", tmp, 1);
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		fflush(stdout);
		return;
	} else if (strncmp(req, "exit", 4) == 0) {
		closedClient(SIGKILL);
	} else if (strncmp(req, "who", 3) == 0) {
		//write(client->clientSendPipe[1], "who\n", 4);
		whoCmd(console.in_fd);
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		return;
		//kill(mainPid, SIGUSR1);
	} else if (strncmp(req, "name ", 5) == 0) {
		write(client->clientSendPipe[1], req, strlen(req));
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		FD_CLR(0,rfds_src);
		return;
		//kill(mainPid, SIGUSR1);
	} else if (strncmp(req, "yell ", 5) == 0) {
		write(client->clientSendPipe[1], req, strlen(req));
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		fflush(stdout);
		FD_CLR(0,rfds_src);
		return;
		//kill(mainPid, SIGUSR1);
	} else if (strncmp(req, "tell ", 5) == 0) {
		write(client->clientSendPipe[1], req, strlen(req));
		closePipe(&queFd[qi]);
		qi++;
		qi %= queMAX;
		fflush(stdout);
		//FD_CLR(0,rfds_src);
		return;
		//kill(mainPid, SIGUSR1);
	}

	int delay = 0;
	has_err_pipe = indexOf(req, "!");
	has_fifoOut = indexOf(req, ">|");
	if (has_fifoOut > 0) {
		req[has_fifoOut + 1] = ' ';
	}else{
		has_fifoOut = indexOf(req, ">!");
		if (has_fifoOut > 0) {
				req[has_fifoOut + 1] = ' ';
				has_err_pipe=-1;
		}
	}

	has_fifoIn = indexOf(req, "<");

	fifoInNumber = -1;
	if (has_fifoIn > 0) {
		if (req[has_fifoIn + 1] == ' ') {
			has_fifoIn = -1;
		} else {
			fifoInNumber = atoi(&req[has_fifoIn + 1]);
		}
	}

#ifdef debug
	if (has_err_pipe != -1)
	dprintf(stdo_fd, "(%d->%d)has error pipe %d\n", getpid(), delay,
			has_err_pipe);
#endif

	Array *arr = split(req, "|!");
	clientCmd->argc = ToArray(arr, &clientCmd->argv);
	Cmd *subCmd = malloc(sizeof(Cmd) * clientCmd->argc);
	Cmd *nextCmd = subCmd;
	Cmd *prevCmd = NULL;
	clientCmd->first = NULL;
	for (i = 0; i < clientCmd->argc; i++) {
		*nextCmd = getCmd(clientCmd->argv[i], " \t");
		if (nextCmd->argc == 0)
			continue;
		if (clientCmd->first == NULL) {
			erro_fd = -1;
			clientCmd->first = nextCmd;
			clientCmd->next = nextCmd;
			if (!isNull(&queFd[qi])) {
				nextCmd->outPipe = queFd[qi];
				//nextCmd->readFd = queFd[qi].out_fd;

				//if(erro_fd==queFd[qi].in_fd)
			}
		}

		if (i == clientCmd->argc - 1 && i != 0) {//final
			delay = atoi(nextCmd->argv[0]);
			if (delay > 0)
				break;
		}

		if (prevCmd) {
			prevCmd->next = nextCmd;
			nextCmd->prev = prevCmd;
		}
		nextCmd->parent = clientCmd;
		nextCmd->first = clientCmd->first;
		prevCmd = nextCmd;
		nextCmd++;
	}

	int dstqi = (qi + delay) % queMAX;
	if (delay > 0) {

		nextCmd = prevCmd;
		if (isNull(&queFd[dstqi]))
			queFd[dstqi] = newPipeFD();
		prevCmd->writeFd = queFd[dstqi].in_fd;
		if (has_err_pipe > 0)
			erro_fd = queFd[dstqi].in_fd;

#ifdef debug
		dprintf(stdo_fd, "(%d->%d)+%d", getpid(), delay, dstqi);
		dprintf(stdo_fd, "i(%d) o(%d)\n", queFd[dstqi].in_fd,
				queFd[dstqi].out_fd);
#endif
	}

	int child_pid;
	nextCmd = clientCmd->first;
	while (nextCmd) {

		child_pid = spawn(nextCmd);

		if (child_pid == 0) //child exec error
		{
			exit(EXIT_SUCCESS);
		} else if (child_pid < 0) {//fork error
			printf("Shell Error may fork too much.\n");
			break;
			//continue;
		}
		if (nextCmd->outPipe.in_fd != console.in_fd) {
			if (isNull(&queFd[(qi + 1) % queMAX])) {
				closeInput(&nextCmd->outPipe);
			} else {
				if (nextCmd->outPipe.in_fd != queFd[(qi + 1) % queMAX].in_fd)
					closeInput(&nextCmd->outPipe);
			}
		}

		int isOver=1;
		if (delay == 0 || nextCmd->next != NULL){
			waitOver(nextCmd->p_id);
			isOver=0;
		}

		if (nextCmd->outPipe.out_fd != console.out_fd) {
			if (isNull(&queFd[dstqi])) {
				closeOutput(&nextCmd->outPipe);
			} else {
				if (nextCmd->outPipe.out_fd != queFd[dstqi].out_fd)
					closeOutput(&nextCmd->outPipe);
			}
		}
		if(isOver){
			waitOver(nextCmd->p_id);
		}
		nextCmd = nextCmd->next;
	}

	//free(subCmd);
	closePipe(&queFd[qi]);
	qi++;
	qi %= queMAX;

}
static void handlerChld(signo){
	if (signo==SIGCHLD){
		int status;
		waitpid(-1,&status,WUNTRACED | WCONTINUED);
	}
}
static void clientInit(int listenSocketFd) {
	clearSignal();
	signal(SIGINT, closedClient);
	signal(SIGKILL, closedClient);
	signal(SIGTERM, closedClient);
        signal(SIGQUIT, closedClient);
	//signal(SIGCHLD, handlerChld);
	close(client->clientSendPipe[0]);
	close(client->mainSendPipe[1]);
	close(0);
	close(1);
	int flags = fcntl(client->conSocketFd, F_GETFL, 0);
	fcntl(client->conSocketFd, F_SETFL, flags | O_NONBLOCK);
	dup(client->conSocketFd);
	dup(client->conSocketFd);
	erro_fd = -1;
	close(listenSocketFd);
	close(stdi_fd);
	memset(&queFd, -1, queMAX * sizeof(PipeFD));
	qi = 0;
	showMotd();
	console.out_fd = dup(client->conSocketFd);
	console.in_fd = dup(client->conSocketFd);
	clientCmd = (Cmd*) malloc(sizeof(Cmd));
	clientCmd->p_id = getpid();
	clientCmd->outPipe = console;
}

static void printfData(int fd_src, int fd_dst) {
	char tmp[1024];
	int len = read(client->mainSendPipe[0], tmp, sizeof(tmp));
	if (len > 0) {
		tmp[len] = '\0';
		dprintf(fd_dst, "%s", tmp);
	}
}
static void clientHandler(int listenSocketFd) {

	clientInit(listenSocketFd);

	fd_set rfds;
	fd_set wfds_src;
	fd_set wfds;
	FD_ZERO(&client_rfds_src);
	FD_SET(0,&client_rfds_src);
	FD_SET(1,&wfds_src);
	FD_SET(client->clientSendPipe[1],&wfds_src);
	FD_SET(client->mainSendPipe[0],&client_rfds_src);
	fflush(stdout);
	int isCanClientReadCmd = 1;
	while (1) {
		bcopy((const void*) &client_rfds_src, (void*) &rfds, sizeof(fd_set));
		bcopy((const void*) &wfds_src, (void*) &wfds, sizeof(fd_set));
		int checkLength = __FD_SETSIZE;
		int s = select(checkLength, &rfds, (fd_set*) 0, (fd_set*) 0,
				(struct timeval*) 0);

		if (FD_ISSET(client->mainSendPipe[0],&rfds)) {
			printfData(client->mainSendPipe[0], 1);
		}
		if (isCanClientReadCmd == 1) {
			write(1, "% ", 2);
			isCanClientReadCmd = 0;
			FD_SET(0,&client_rfds_src);
		} else if (FD_ISSET(0,&rfds)) {

			readClientCmd(&client_rfds_src);
			isCanClientReadCmd = 1;

		}
		if (isCanClientReadCmd == 1 && FD_ISSET(0,&client_rfds_src)) {
			write(1, "% ", 2);
			isCanClientReadCmd = 0;
			//FD_SET(0,&rfds_src);
		}

	}
	closedClient(SIGKILL);
}

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
		client->remote_port =ntohs( dest.sin_port);
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

int listenSocketFd;
static int killClient(Client* c) {
	if (c->pid > 0) {
		kill(c->pid, SIGKILL);

	waitOver(c->pid);
	beginUpdateClientQueue();
	getClientQueue();
	c->pid = 0;
	shutdown(c->conSocketFd, 2);
	close(c->clientSendPipe[0]);
	//close(c->clientSendPipe[1]);
	//close(c->mainSendPipe[0]);
	close(c->mainSendPipe[1]);
	memset(c, 0, sizeof(Client));
	endUpdateClientQueue();
        }
	return 0;
}
static void readCmd() {
	char line[255];
	fgets(line, 255, stdin);
	if (strcmp(line, "who\n") == 0) {
		whoCmd(stdo_fd);
	} else if (strcmp(line, "kill\n") == 0) {
	//	killClient(&clientQueue[0]);
	}
	//printf("sss:%s",input);
}
static void closedMain() {
	allClientAction(killClient);
	close(listenSocketFd);
	exit(EXIT_SUCCESS);

}
void mainClientSignHandler(int signo) {

	//int oldmask = sigblock(sigmask(SIGUSR1));
	if (signo == SIGUSR1) {
		dprintf(stdo_fd, "user1\n");
	}
	//sigsetmask(oldmask);
        if (signo == SIGPIPE){
		dprintf(stdo_fd, "pipe broke%d\n",getpid());
        }
	if (signo == SIGCHLD) {
		dprintf(stdo_fd, "kill%d \n", getpid());
	        //kill(getpid(), SIGUSR1);
	}

}

static void clientCastMsg(char *msg, int selfIndex) {
    lock();
    getClientQueue();
	int i;
	for (i = 0; i < ClientQueueLength; i++) {
		Client *c = &clientQueue[i];
		if (c->pid > 0) {
			if (selfIndex != i) {
				if (strlen(msg) > 0)
					dprintf(c->mainSendPipe[1], "%s", msg);
			}
		}
	}
	unlock();
}
static void checkAllClientRpipe(fd_set* rfds_src, fd_set* rfds) {
	int i;
	char msg[255];
	lock();
	getClientQueue();
	unlock();
	Client *c;
	for (i = 0; i < ClientQueueLength; i++) {
		c = &clientQueue[i];
		if (c->pid > 0) {
			if (FD_ISSET(c->clientSendPipe[0],rfds)) {
				char tmp[1024];
				int len = read(c->clientSendPipe[0], tmp, 1024);
				if (len > 0) {
					tmp[len] = '\0';
					/*if(strncmp("who",tmp,3)==0){
					 whoCmd(c->mainSendPipe[1]);
					 dprintf(stdo_fd, "%s", tmp);
					 }else
					 */

					if (strncmp("name ", tmp, 5) == 0) {
						//sscanf(tmp,"name %s\n",c->name);
						sprintf(c->name, "%s", &tmp[5]);
						setClientQueue();
						sprintf(msg,
								"*** User from %s/%d is named '%s'. ***\r\n",
								c->remote_ip, c->remote_port, c->name);
						clientCastMsg(msg, -1);
						dprintf(stdo_fd, "%s", msg);
					} else if (strncmp("yell ", tmp, 5) == 0) {
						sprintf(msg, "*** %s yelled ***: %s\n", c->name,
								&tmp[5]);
						clientCastMsg(msg, -1);
						dprintf(stdo_fd, "%s", msg);
					} else if (strncmp("tell ", tmp, 5) == 0) {
						tmp[len] = '\n';
						int i = atoi(&tmp[5]);
						if (i > 0 && i <= ClientQueueLength) {
							char pmsg[255];
							memset(pmsg,0,255);
							sscanf(tmp, "tell %d %[^\n]", &i, pmsg);
							int index = i - 1;
							if (clientQueue[index].pid > 0) {
								sprintf(msg, "*** %s told you ***: %s\n",
										c->name, pmsg);
								dprintf(clientQueue[index].mainSendPipe[1],
										"%s", msg);
								//dprintf(stdo_fd, "%s", tmp);
							} else {
								dprintf(
										c->mainSendPipe[1],
										"*** Error: user #%d does not exist yet. ***\n",
										i);
							}

						} else {
							dprintf(
									c->mainSendPipe[1],
									"*** Error: user #%d does not exist yet. ***\n",
									i);
						}
					}else if (strncmp("fifo_w", tmp, 6) == 0) {
						char ccmd[255];
						sscanf(tmp, "fifo_w %[^\n]", ccmd);
						sprintf(msg, "*** %s (#%d) just piped '%s' into his/her pipe. ***\n", c->name,
								c->index+1,ccmd);
						clientCastMsg(msg, -1);
						dprintf(stdo_fd, "%s", msg);
					}else if (strncmp("fifo_r", tmp, 6) == 0) {
						char ccmd[255];
						int revice_i;
						sscanf(tmp, "fifo_r %d %[^\n]",&revice_i, ccmd);
						Client *c2=&clientQueue[revice_i];
						sprintf(msg, "*** %s (#%d) just received the pipe from %s (#%d) by '%s' ***\n", c->name,
								c->index+1,c2->name,revice_i+1,ccmd);
						clientCastMsg(msg, -1);
						dprintf(stdo_fd, "%s", msg);
					}


				} else {
					dprintf(stdo_fd, "%s client exit", c->name);
					sprintf(msg, "*** User '%s' left. ***\n", c->name);
					clientCastMsg(msg, i);
					FD_CLR(c->clientSendPipe[0],rfds_src);
					killClient(c);
				}
			}
		}
	}
}
fd_set main_rfds_src;
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
	beginUpdateClientQueue();
	InitClientQueue();
	endUpdateClientQueue();
	printf("Input Server Port:");
	char input[10];
	fgets(input, 9, stdin);
	if (atoi(input) != 0)
		PORTNUM = atoi(input);

	stdi_fd = dup(0);
	stdo_fd = dup(1);
	mainPid = getpid();

	memset(&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = INADDR_ANY;
	serv.sin_port = htons(PORTNUM);

	listenSocketFd = socket(AF_INET, SOCK_STREAM, 0);
	int flags = fcntl(listenSocketFd, F_GETFL, 0);
	fcntl(listenSocketFd, F_SETFL, flags | O_NONBLOCK);
	bind(listenSocketFd, (struct sockaddr *) &serv, sizeof(struct sockaddr));
	listen(listenSocketFd, 5);
	signal(SIGKILL, SIG_DFL);
	signal(SIGINT, SIG_DFL);
	signal(SIGTERM, SIG_DFL);
	signal(SIGKILL, closedMain);
	signal(SIGINT, closedMain);
	signal(SIGTERM, closedMain);
	signal(SIGQUIT, closedMain);
	signal(SIGUSR1, mainClientSignHandler);
	signal(SIGCHLD, mainClientSignHandler);
	signal(SIGPIPE,mainClientSignHandler);

	fd_set rfds;
	FD_ZERO(&main_rfds_src);
	FD_SET(0,&main_rfds_src);
	FD_SET(listenSocketFd,&main_rfds_src);

	while (1) {
		dprintf(stdo_fd, "# ");
		bcopy((const void*) &main_rfds_src, (void*) &rfds, sizeof(fd_set));
		int checkLength = __FD_SETSIZE;
		int s = select(checkLength, &rfds, (fd_set*) 0, (fd_set*) 0,
				(struct timeval*) 0);
		if (s == -1) {
			continue;
		}

		if (FD_ISSET(listenSocketFd,&rfds)) {
			int index = acceptClient(listenSocketFd);
			Client *c = &clientQueue[index];
			char msg[255];
			sprintf(msg, "*** User '%s' entered from %s/%d. ***\n", c->name,
					c->remote_ip, c->remote_port);
			clientCastMsg(msg, -1);
			FD_SET(c->clientSendPipe[0],&main_rfds_src);
		}

		if (FD_ISSET(0,&rfds)) {
			readCmd();
		}
		checkAllClientRpipe(&main_rfds_src, &rfds);
	}

	dprintf(stdo_fd, "server stop\n");
	close(listenSocketFd);
	exit(EXIT_SUCCESS);
}

