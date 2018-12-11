CC = gcc
CFLAGS = -g -Wall

all: main

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

proxy.o: main.c csapp.h
	$(CC) $(CFLAGS) -c main.c

main: main.o csapp.o
	$(CC) $(CFLAGS) main.o csapp.o -o main

clean:
	rm -f *.o proxy
