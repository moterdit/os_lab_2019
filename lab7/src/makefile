CC=gcc
CFLAGS=-I.
LDFLAGS=-lpthread

TARGETS=tcpclient tcpserver udpclient udpserver

tcpclient :
	$(CC) $(CFLAGS) -o tcpclient tcpclient.c $(LDFLAGS)

tcpserver :
	$(CC) $(CFLAGS) -o tcpserver tcpserver.c $(LDFLAGS)

udpclient :
	$(CC) $(CFLAGS) -o udpclient udpclient.c $(LDFLAGS)

udpserver :
	$(CC) $(CFLAGS) -o udpserver udpserver.c $(LDFLAGS)

clean :
	rm tcpclient tcpserver udpclient udpserver

all: $(TARGETS)