all:
	gcc -Wall -c common.c -lm
	gcc -Wall client.c common.o -o bin/client -lm
	gcc -Wall server.c common.o -o bin/server -lm