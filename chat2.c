#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#define MAX_CLIENTS 50
#define BUFFER_SIZE 2048
#define NAME_SIZE 32
#define PORT 8080

typedef struct {
    int socket;
    char name[NAME_SIZE];
    struct sockaddr_in address;
} client_t;

// Variables globales pour la gestion des clients
client_t *clients[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
int client_count = 0;

// Fonctions utilitaires
void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket != sender_socket) {
            if (send(clients[i]->socket, message, strlen(message), 0) < 0) {
                perror("Erreur d'envoi du message");
            }
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void remove_client(int socket) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != NULL && clients[i]->socket == socket) {
            free(clients[i]);
            clients[i] = NULL;
            client_count--;
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

void add_client(client_t *client) {
    pthread_mutex_lock(&clients_mutex);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL) {
            clients[i] = client;
            client_count++;
            break;
        }
    }
    
    pthread_mutex_unlock(&clients_mutex);
}

// Fonction de gestion d'un client
void *handle_client(void *arg) {
    client_t *client = (client_t *)arg;
    char buffer[BUFFER_SIZE];
    char message[BUFFER_SIZE + NAME_SIZE + 4];  // Extra space for name and formatting
    
    // Demander le nom du client
    send(client->socket, "Entrez votre nom: ", 18, 0);
    ssize_t name_len = recv(client->socket, client->name, NAME_SIZE - 1, 0);
    if (name_len <= 0) {
        close(client->socket);
        remove_client(client->socket);
        pthread_exit(NULL);
    }
    client->name[strcspn(client->name, "\n")] = 0;
    client->name[strcspn(client->name, "\r")] = 0;
    
    // Annoncer l'arrivée du nouveau client
    snprintf(message, sizeof(message), "--- %s a rejoint le chat ---\n", client->name);
    broadcast_message(message, client->socket);
    printf("%s", message);
    
    // Boucle principale de chat
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        ssize_t recv_len = recv(client->socket, buffer, BUFFER_SIZE - 1, 0);
        
        if (recv_len <= 0) {
            // Client déconnecté
            snprintf(message, sizeof(message), "--- %s a quitté le chat ---\n", client->name);
            broadcast_message(message, client->socket);
            printf("%s", message);
            break;
        }
        
        buffer[strcspn(buffer, "\n")] = 0;
        buffer[strcspn(buffer, "\r")] = 0;
        
        // Commande pour quitter
        if (strcmp(buffer, "/quit") == 0) {
            snprintf(message, sizeof(message), "--- %s a quitté le chat ---\n", client->name);
            broadcast_message(message, client->socket);
            printf("%s", message);
            break;
        }
        
        // Diffuser le message
        snprintf(message, sizeof(message), "%s: %s\n", client->name, buffer);
        broadcast_message(message, -1);  // -1 pour envoyer à tous, y compris l'expéditeur
        printf("%s", message);
    }
    
    close(client->socket);
    remove_client(client->socket);
    pthread_exit(NULL);
}

int main() {
    int server_socket;
    struct sockaddr_in server_addr;
    pthread_t thread;
    
    // Ignorer SIGPIPE
    signal(SIGPIPE, SIG_IGN);
    
    // Création du socket serveur
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Erreur création socket");
        exit(EXIT_FAILURE);
    }
    
    // Configuration de l'adresse du serveur
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    
    // Option de réutilisation d'adresse
    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Erreur setsockopt");
        exit(EXIT_FAILURE);
    }
    
    // Bind
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Erreur bind");
        exit(EXIT_FAILURE);
    }
    
    // Listen
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Erreur listen");
        exit(EXIT_FAILURE);
    }
    
    printf("Serveur de chat démarré sur le port %d\n", PORT);
    
    // Boucle principale d'acceptation des clients
    while (1) {
        client_t *client = malloc(sizeof(client_t));
        socklen_t client_len = sizeof(client->address);
        
        client->socket = accept(server_socket, (struct sockaddr *)&client->address, &client_len);
        
        if (client->socket < 0) {
            perror("Erreur accept");
            free(client);
            continue;
        }
        
        printf("Nouvelle connexion depuis %s:%d\n",
               inet_ntoa(client->address.sin_addr),
               ntohs(client->address.sin_port));
        
        if (client_count >= MAX_CLIENTS) {
            printf("Nombre maximum de clients atteint. Connexion refusée.\n");
            send(client->socket, "Serveur plein. Réessayez plus tard.\n", 36, 0);
            close(client->socket);
            free(client);
            continue;
        }
        
        // Création d'un thread pour gérer le client
        if (pthread_create(&thread, NULL, handle_client, (void *)client) != 0) {
            perror("Erreur création thread");
            close(client->socket);
            free(client);
            continue;
        }
        
        // Détachement du thread
        pthread_detach(thread);
        add_client(client);
    }
    
    close(server_socket);
    return EXIT_SUCCESS;
}
