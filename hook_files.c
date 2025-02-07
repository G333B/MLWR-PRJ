#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>  
#include <errno.h>   
#include <stdarg.h>  // Ajouté pour gérer les arguments variables

// Pointeur vers la vraie fonction open
static int (*real_open)(const char *pathname, int flags, ...) = NULL;

void init() {
    if (!real_open) {
        real_open = dlsym(RTLD_NEXT, "open");
    }
}

// Hook de open() pour empêcher l'ouverture de certains fichiers
int open(const char *pathname, int flags, ...) {
    init();

    // Liste de fichiers à bloquer
    const char *blocked_file = "/tmp/secret.txt";

    if (strstr(pathname, blocked_file) != NULL) {
        // Log le blocage de l'ouverture du fichier
        FILE *logfile = fopen("/tmp/.ssh_logs", "a");
        if (logfile) {
            fprintf(logfile, "[BLOCK] Attempt to open blocked file: %s\n", pathname);
            fclose(logfile);
        }

        // Empêcher l'ouverture du fichier
        errno = EACCES;  
        return -1;
    }

    // Gérer le mode facultatif
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);

    // Appeler la vraie fonction open avec les bons arguments
    return real_open(pathname, flags, mode);
}