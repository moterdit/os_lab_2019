CC=gcc
CFLAGS=-I.


multmodulo.so : multmodulo.h 
	$(CC) -c -fPIC multmodulo.c -o libmultmodulo.o
	$(CC) -o libmultmodulo.so -shared libmultmodulo.o

server: multmodulo.so
	$(CC) -c server.c -o server.o
	$(CC) -o server server.o -L$(shell pwd) -lmultmodulo -pthread -Wl,-rpath=$(shell pwd) $(CFLAGS)

client: multmodulo.so
	$(CC) -c client.c -o client.o
	$(CC) -o client client.o -L$(shell pwd) -lmultmodulo -pthread -Wl,-rpath=$(shell pwd) $(CFLAGS)

all: multmodulo.so client server

clean:
	rm client client.o server server.o libmultmodulo.o libmultmodulo.so