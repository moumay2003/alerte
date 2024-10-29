#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 2014
#define MAX_NAME 50

int main() {
    int dsc, dss;
    int i = 0;
    int listuser[MAX_CLIENTS];
    char *reqt = calloc(BUFFER_SIZE, sizeof(char));
    char *reps = calloc(BUFFER_SIZE, sizeof(char));
    char *pseudo = calloc(MAX_NAME, sizeof(char));
    char *message = calloc(BUFFER_SIZE + MAX_NAME, sizeof(char));
    
    struct sockaddr_in addSrv, addCLT;
    socklen_t lgAdrCLT = sizeof(struct sockaddr);
    
    // Configuration du serveur
    addSrv.sin_family = AF_INET;
    addSrv.sin_port = htons(8080);
    inet_aton("127.0.0.1", &(addSrv.sin_addr));
    memset(&addSrv.sin_zero, 0, 8);
    
    // Création du socket
    dss = socket(AF_INET, SOCK_STREAM, 0);
    printf("dss : %d\n", dss);
    
    // Bind
    if (bind(dss, (struct sockaddr *)&addSrv, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    
    // Listen
    if (listen(dss, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(1);
    }
    
    printf("Démarrage du serveur de chat\n");
    
    while(1) {
        dsc = accept(dss, (struct sockaddr *)&addCLT, &lgAdrCLT);
        printf("Nouveau client connecté au serveur\n");
        
        if (fork() != 0) {
            // Processus parent
            close(dsc);
        } else {
            // Processus enfant
            close(dss);
            
            // Demande du pseudo
            strcpy(reps, "Entrez votre pseudo: ");
            send(dsc, reps, strlen(reps), 0);
            recv(dsc, pseudo, MAX_NAME, 0);
            pseudo[strcspn(pseudo, "\n")] = '\0';
            pseudo[strcspn(pseudo, "\r")] = '\0';
            
            // Ajout du client à la liste
            listuser[i] = dsc;
            i++;
            
            // Message de bienvenue
            snprintf(message, BUFFER_SIZE + MAX_NAME, "\n>>> %s a rejoint le chat <<<\n", pseudo);
            for (int j = 0; j < i; j++) {
                send(listuser[j], message, strlen(message), 0);
            }
            
            while(1) {
                memset(reqt, '\0', BUFFER_SIZE);
                memset(message, '\0', BUFFER_SIZE + MAX_NAME);
                
                // Réception du message
                int recv_ret = recv(dsc, reqt, BUFFER_SIZE, 0);
                if (recv_ret <= 0) {
                    break;
                }
                
                reqt[strcspn(reqt, "\r")] = '\0';
                reqt[strcspn(reqt, "\n")] = '\0';
                
                // Vérification si le client veut quitter
                if (strcmp(reqt, "exit") == 0) break;
                
                // Formatage du message avec le pseudo
                snprintf(message, BUFFER_SIZE + MAX_NAME, "%s: %s\n", pseudo, reqt);
                
                // Envoi du message à tous les clients
                for (int j = 0; j < i; j++) {
                    if (listuser[j] != dsc) {  // Ne pas renvoyer au client qui a envoyé
                        send(listuser[j], message, strlen(message), 0);
                    }
                }
            }
            
            // Message de déconnexion
            snprintf(message, BUFFER_SIZE + MAX_NAME, "\n>>> %s a quitté le chat <<<\n", pseudo);
            for (int j = 0; j < i; j++) {
                if (listuser[j] != dsc) {
                    send(listuser[j], message, strlen(message), 0);
                }
            }
            
            close(dsc);
            printf("Client %s déconnecté\n", pseudo);
            exit(0);
        }
    }
    
    // Libération de la mémoire
    free(reqt);
    free(reps);
    free(pseudo);
    free(message);
    close(dss);
    
    return EXIT_SUCCESS;
}
