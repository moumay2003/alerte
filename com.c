#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>


 int main() {
 int dsc ,dss ;
  char *reqt=calloc(2014,sizeof(char)) ;   char *reps=calloc(2014,sizeof(char)) ;

  struct sockaddr_in addSrv ,addCLT;
  socklen_t lgAdrCLT=sizeof(struct sockaddr);
  addSrv.sin_family=AF_INET;
  addSrv.sin_port=htons(8080);
  inet_aton("127.0.0.1",&(addSrv.sin_addr));
  memset(&addSrv.sin_zero,0,8);
  dss=socket(AF_INET,SOCK_STREAM,0);
  printf("dss : %d\n",dss);
  if ( bind(dss,(struct sockaddr *)&addSrv,sizeof(struct sockaddr))==-1) {
    perror("bind");
    exit(1);
  }
  if(listen(dss,10)==-1) {
    perror("listen");
    exit(1);
  }
    printf ("demarrage du server\n");

  while(1){
    dsc=accept(dss,(struct sockaddr *)&addCLT,&lgAdrCLT);
    printf ("nouveau clien connecté du server\n");
    while(1){
        memset(reqt,'\0',2014);
        recv(dsc,reqt,2014,0);
        strcpy(reps,reqt);
        send(dsc,reps,strlen(reps),0);
        reqt[strcspn(reqt,"\r")]='\0';  reqt[strcspn(reqt,"\n")]='\0';
        if(strcmp(reqt,"exit")==0) break;


    }
     close(dsc);
     printf("client deconnecté\n");

  }
   close(dss);
   return EXIT_SUCCESS;
  
  

 }