OBJECTS = main.o
CC = gcc
FLAGS = `pkg-config --cflags --libs glib-2.0` -g -Wall -Werror

all : main test

main : main.c 
	$(CC) $(FLAGS)  main.c -o nit

test : test.c
	$(CC) $(FLAGS) test.c -o run_these_tests
