#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>

#define SEMKEY 2L
#define PERMS 0666

static struct sembuf op_lock[2] = { { 0, 0, 0 }, { 0, 1, SEM_UNDO } };
static struct sembuf op_unlock[1] = { { 0, -1, (IPC_NOWAIT | SEM_UNDO) } };

int sem_id = -1;
void lock() {
	if (sem_id < 0) {
		sem_id = semget(SEMKEY, 1, IPC_CREAT | PERMS);
	}
	semop(sem_id, &op_lock[0], 2);
}

void unlock() {
	semop(sem_id, &op_unlock[0], 1);
}
#define MAXMESGDATA 1024
#define MEGHDRESIZE (sizof(Mesg)-MAXMESGDATA)
typedef struct {
	int mesg_len;
	long mesg_type;
	char mesg_data[MAXMESGDATA];
}Mesg;
Mesg *mesgptr;
#define SHMKEY 11

void server(){
	lock();
	char buf[255];
	int shmid=shmget((key_t)SHMKEY,(size_t)sizeof(Mesg),0666|IPC_CREAT);
	printf("(%d)server:",shmid);
	mesgptr=(Mesg*)shmat(shmid,(char *)0,0);
	fgets(buf,255,stdin);
	strcpy(mesgptr->mesg_data,buf);
	unlock();
}


void client(){
	lock();
	int shmid=shmget((key_t)SHMKEY,(size_t)sizeof(Mesg),0666|IPC_CREAT);
	mesgptr=(Mesg*)shmat(shmid,(char *)0,0);
	printf("(%d)client rev:%s",shmid,mesgptr->mesg_data);
	unlock();
}

int funFork(void (*func)()) {
	int node_id = 0;
	node_id = fork();
	if (node_id != 0) {//root
		return node_id;
	} else {
		func();
	}
	return 0;
}

void waitOver(int pid) {
	int status;
	do {
		if (waitpid(pid, &status, WUNTRACED | WCONTINUED) == -1)
			break;
	} while (!
	WIFEXITED(status) && !WIFSIGNALED(status));
}

int main(int argc, char *argv[], char *envp[]) {
	int serverId, clientId = 0;
	serverId = funFork(&server);
	if (serverId == 0) {
		printf("pid(%d) end\n", getpid());
		return 0;
	}
	clientId = funFork(&client);
	if (clientId == 0) {
		printf("pid(%d) end\n", getpid());
		return 0;
	}
	waitOver(serverId);
	waitOver(clientId);
	int shmid=shmget((key_t)SHMKEY,(size_t)sizeof(Mesg),0666|IPC_CREAT);
	mesgptr=(Mesg*)shmat(shmid,(char *)0,0);
	printf("(%d)main rev:%s",shmid,mesgptr->mesg_data);
	printf("main end\n");
	return 0;
}
