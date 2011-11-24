#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <memory.h>
#include <time.h>
/*

#include <sys/select.h>

// According to earlier standards 
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
  fd_set *exceptfds, struct timeval *timeout);

void FD_CLR(int fd, fd_set *set);
)t  FD_ISSET(int fd, fd_set *set);
void FD_SET(int fd, fd_set *set);
void FD_ZERO(fd_set *set);

#include <sys/select.h>
int pselect(int nfds, fd_set *readfds, fd_set *writefds,
  fd_set *exceptfds, const struct timespec *timeout,
  const sigset_t *sigmask);
*/
void showBit(fd_set fds,int index){
   int i;
   for (i=0;i<8;i++){
      printf("%d",FD_ISSET(index*8+i,&fds));
   }
}
int main(int argc,char *argv[],char *envp[])
{
   fd_set fds;
   fd_set rfds_src;
   fd_set rfds;
   fd_set wfds_src;
   fd_set wfds;
   int pfd[2];
   pipe(pfd);
   int p_in=pfd[0];
   int p_out=pfd[1];
   printf("fd_set size=%d\n",sizeof(fds));
   int fd_stdin=0;
   int fd_stdout=1;
   int fd_stderr=2;

   FD_ZERO(&fds);//init
   FD_ZERO(&rfds_src);//init
   FD_ZERO(&wfds_src);//init
   printf("fds=");
   showBit(fds,0);
   printf("\n");

   FD_SET(p_in,&fds); 
   FD_SET(p_out,&fds); 
   printf("fds=");
   showBit(fds,0);
   printf("\n--read start--\n");
   int checkLength=10;
   bcopy((char*)&fds,(char*)&rfds_src,sizeof(fd_set));
   bcopy((char*)&fds,(char*)&wfds_src,sizeof(fd_set));
   FD_SET(fd_stdin,&rfds_src); //fds=1000
   FD_SET(fd_stdout,&wfds_src); //fds=1010
   while(1){
      char buf[10240];
      int s=0;
      bcopy((char*)&rfds_src,(char*)&rfds,sizeof(fd_set));
      bcopy((char*)&wfds_src,(char*)&wfds,sizeof(fd_set));

      printf("---start select ---\n");
      s=select(checkLength,&rfds,&wfds,(fd_set*)0,(struct timeval*)0);
      printf("rfds=");
      showBit(rfds,0);
      printf(" wfds=");
      showBit(wfds,0);
      printf("\n");
      printf("---return (%d)---\n",s);
      fflush(stdout);

      if(FD_ISSET(fd_stdin,&rfds)){
         int len=read(fd_stdin,buf,sizeof(buf));
         printf("input len(%d)\n",len);
         buf[len]='\0';
         if(FD_ISSET(p_out,&wfds)){
            dprintf(p_out,"%s",buf);
         }
      }

      if(FD_ISSET(p_in,&rfds)){
         if(!FD_ISSET(p_out,&wfds)){
         int len=read(p_in,buf,sizeof(buf));
         printf("out len(%d)\n",len);
         }
      }
      sleep(1);
      system("clear");

   }
   return 0;
}
