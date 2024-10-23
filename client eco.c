#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include<time.h>
int main() {
    int ds;
   
    char *reqt = calloc(2014, sizeof(char));
    char *rep = calloc(1024, sizeof(char));
    struct sockaddr_in addser;
    socklen_t lgAdrCLT = sizeof(struct sockaddr);
    
    addser.sin_family = AF_INET;
    addser.sin_port = htons(8080);
    inet_aton("127.0.0.1", &(addser.sin_addr));
    memset(&addser.sin_zero, 0, 8);
    ds=socket(AF_INET, SOCK_STREAM, 0);
    connect(ds, (struct sockaddr *)&addser, sizeof(struct sockaddr));
    while(1)
    {
        fgets(reqt, 2014, stdin);
        send(ds, reqt, strlen(reqt), 0);
        recv(ds,rep,1024,0);
        printf("Message recu est :%s\n",rep);
        reqt[strcspn(reqt, "\n")] = '\0';
         if (strcmp(reqt, "exit") == 0)
                break;
        memset(rep, '\0', 1024);
        memset(reqt, '\0', 2014);

     }  
    close(ds);
    return 0;
}  