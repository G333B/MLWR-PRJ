#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>
#include <fcntl.h>  
#include <errno.h>   
#include <stdarg.h>  

// Pointeur vers fonction open
static int (*real_open)(const char *pathname, int flags, ...) = NULL;

void init() {
    if (!real_open) {
        real_open = dlsym(RTLD_NEXT, "open");
    }
}

// open() pour empêcher l'ouverture du fichier /tmp/secret.txt
int open(const char *pathname, int flags, ...) {
    init();

    
    const char *blocked_file = "/tmp/secret.txt";

    if (strstr(pathname, blocked_file) != NULL) {
        
        FILE *logfile = fopen("/tmp/.ssh_logs", "a");
        if (logfile) {
            fprintf(logfile, "[BLOCK] Attempt to open blocked file: %s\n", pathname);
            fclose(logfile);
        }

        // Empêcher l'ouverture du fichier
        errno = EACCES;  
        return -1;
    }

    
    va_list args;
    va_start(args, flags);
    mode_t mode = va_arg(args, mode_t);
    va_end(args);

    // Appeler de la fonction open() originale
    return real_open(pathname, flags, mode);
}