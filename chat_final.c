#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netinet/in.h>


#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int main() {
    int server_socket, client_socket, client_count = 0;
    int clients[MAX_CLIENTS];
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

    // Création du socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Erreur de création du socket");
        exit(EXIT_FAILURE);
    }

    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8082);
    inet_aton("127.0.0.1", &(server_addr.sin_addr));
    memset(&server_addr.sin_zero, 0, 8);

    // Liaison du socket à l'adresse et au port
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur de liaison");
        
        exit(EXIT_FAILURE);
    }

    // Écouter les connexions entrantes
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur d'écoute");
        
        exit(EXIT_FAILURE);
    }
    printf("Serveur de chat en écoute sur le port %d\n", 8082);

    // Accepter et gérer les connexions des clients
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len)) >= 0) {
        printf("Nouveau client connecté.\n");

        // Ajouter le client à la liste des clients connectés
        clients[client_count++] = client_socket;

        // Créer un processus enfant pour gérer le client
        if (fork() == 0) {
            // Processus enfant: gérer le client
            close(server_socket);  // Fermer le socket du serveur dans le processus enfant

            // Envoi du message de bienvenue au client
            char *welcome_message = "Bienvenue sur le serveur de chat!\n";
            send(client_socket, welcome_message, strlen(welcome_message), 0);

            // Boucle de lecture des messages du client
            int read_size;
            while ((read_size = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
                buffer[read_size] = '\0';  // Ajouter le caractère de fin de chaîne
                printf("Message reçu: %s", buffer);

                // Diffuser le message aux autres clients
                for (int i = 0; i < client_count; i++) {
                    if (clients[i] != client_socket) {
                        send(clients[i], buffer, strlen(buffer), 0);
                    }
                }
            }

            // Déconnexion du client
            printf("Client déconnecté.\n");
            close(client_socket);

            // Retirer le client de la liste
            for (int i = 0; i < client_count; i++) {
                if (clients[i] == client_socket) {
                    clients[i] = clients[client_count - 1];
                    client_count--;
                    break;
                }
            }

            exit(0);  // Terminer le processus enfant
        }
    }

    // Fermer le socket du serveur
    close(server_socket);

    return 0;
}
