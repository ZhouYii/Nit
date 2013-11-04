OBJECTS = main.o
CC = gcc
FLAGS = `pkg-config --cflags --libs glib-2.0` -g -Wall -Werror

all : main 

main : main.c 
	$(CC) $(FLAGS)  main.c -o nit

