CC = gcc

all: main

csapp.o: csapp.c csapp.h
	$(CC) -c csapp.c

proxy.o: main.c csapp.h
	$(CC) -c main.c

main: main.o csapp.o
	$(CC) main.o csapp.o -o main

clean:
	rm -f *.o proxy
