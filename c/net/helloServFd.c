#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define PORTNUM 8000

int main(int argc,char *argv[],char *envp[]){
   struct sockaddr_in dest; /* socket info about the machine connecting to us */
   struct sockaddr_in serv; /* socket info about our server */
   int mysocket;            /* socket used to listen for incoming connections */
   int socksize = sizeof(struct sockaddr_in);

   memset(&serv, 0, sizeof(serv));    /* zero the struct before filling the fields */
   serv.sin_family = AF_INET;         /* set the type of connection to TCP/IP */
   serv.sin_addr.s_addr = INADDR_ANY; /* set our address to any interface */
   serv.sin_port = htons(PORTNUM);    /* set the server port number */    

   mysocket = socket(AF_INET, SOCK_STREAM, 0);

   /* bind serv information to mysocket */
   bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

   /* start listening, allowing a queue of up to 1 pending connection */
   listen(mysocket, 1);
   int node_id=0;
   int stdo_fd=dup(0);
   int stdi_fd=dup(1);
   while(mysocket){
         int consocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
         if(consocket<0)
            break;
         node_id=fork();
         if(node_id!=0){
            close(consocket);
            continue;
         }
         else{
            char *remote_ip=inet_ntoa(dest.sin_addr);
            close(1);
            dup(consocket);

            close(0);
            dup(consocket);
            dprintf(stdo_fd,"%s open\n",remote_ip);
            while(consocket)
            {
               char msg[255];
               char buffer[120];
               int len=0;
               memset(buffer,0,120);
               //len = recv(consocket, buffer,120 , 0);
               if(!fgets(buffer,120,stdin)){
                  dprintf(stdo_fd,"%s close\n",remote_ip);
                  break;
               }
              char *req;
              req=strtok(buffer,"\r|\n");
              if(!req)
                 continue;
              len=strlen(req);

              sprintf(msg,"--Received--\n%s\nEnd(%d bytes).\n",req, len);
                  //write(stdo_fd,buffer,strlen(buffer));
                  dprintf(stdo_fd,"%d\n",req[0]);
                  dprintf(stdo_fd,"%s",msg);

                  //printf("%s",msg);
                  //send(consocket, msg, strlen(msg), 0); 
                  dprintf(consocket,"%s",msg);
            }
            close(consocket);
            close(1);
            close(0);
            break;
         }
         break;
   }

   close(mysocket);
}
