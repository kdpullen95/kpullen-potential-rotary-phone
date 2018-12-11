CC = gcc
CFLAGS = -g -Wall
LDFLAGS = -lpthread

all: main

csapp.o: csapp.c csapp.h
	$(CC) $(CFLAGS) -c csapp.c

main.o: main.c csapp.h
	$(CC) $(CFLAGS) -c main.c

main: main.o csapp.o
	$(CC) $(CFLAGS) main.o csapp.o -o main $(LDFLAGS)

clean:
	rm -f *~ *.o main
