CC=gcc
CFLAGS=-I.

client: client.c utils.c parson.c
	$(CC) -o client -g client.c utils.c parson.c -Wall

run: client
	./client

clean:
	rm -f *.o client
