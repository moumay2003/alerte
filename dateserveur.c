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
    int dss, dsc;
    time_t ladate;
    time(&ladate);
    struct tm ladateFmt = *localtime(&ladate);
    char *reqt = calloc(2014, sizeof(char));
    char *rep = calloc(1024, sizeof(char));
    struct sockaddr_in addser, addclt;
    socklen_t lgAdrCLT = sizeof(struct sockaddr);
    
    addser.sin_family = AF_INET;
    addser.sin_port = htons(8080);
    inet_aton("127.0.0.1", &(addser.sin_addr));
    memset(&addser.sin_zero, 0, 8);
    
    // Création du socket
    dss = socket(AF_INET, SOCK_STREAM, 0);
    if (dss == -1) {
        perror("socket");
        exit(1);
    }
    printf("dss : %d\n", dss);
    
    // Liaison du socket avec l'adresse
    if (bind(dss, (struct sockaddr *)&addser, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }
    
    // Écoute des connexions entrantes
    if (listen(dss, 10) == -1) {
        perror("listen");
        exit(1);
    }
    printf("Serveur démarré, en attente de clients...\n");
    
    // Boucle principale pour accepter des connexions
    while (1) {
        dsc = accept(dss, (struct sockaddr *)&addclt, &lgAdrCLT);
        if (dsc == -1) {
            perror("accept");
            continue;
        }
        printf("Nouveau client connecté au serveur\n");
        
        while (1) {
            memset(reqt, '\0', 2014);
            recv(dsc, reqt, 2014, 0);
            
            reqt[strcspn(reqt, "\r\n")] = '\0';  // Supprimer les caractères de nouvelle ligne
            
            if (strcmp(reqt, "DATE") == 0) {
                // Répondre avec la date actuelle
                strcpy(rep, asctime(&ladateFmt));
                send(dsc, rep, strlen(rep), 0);
            }
            if (strcmp(reqt, "exit") == 0) {
                break; // Quitter la boucle si "exit" est reçu
            }
        }
        
        close(dsc);  // Fermer la connexion avec le client
        printf("Client déconnecté\n");
    }
    
    // Fermeture du socket principal et libération de la mémoire
    close(dss);
    free(reqt);
    free(rep);
    
    return EXIT_SUCCESS;
}
