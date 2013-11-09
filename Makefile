OBJECTS = main.o
CC = gcc
FLAGS = `pkg-config --cflags --libs glib-2.0` -g -Wall -Werror

all : main test server client

main : main.c
	$(CC) $(FLAGS)  main.c -o nit

test : test.c
	$(CC) $(FLAGS) test.c -o run_these_tests

server : server.c
	$(CC) $(FLAGS) server.c -o server

client : client.c
	$(CC) $(FLAGS) client.c -o client
