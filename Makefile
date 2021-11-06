CC = gcc

CFLAGS = -w

LFLAGS = -lSDL2

all: isogame

isogame: isogame.c
	$(CC) isogame.c $(CFLAGS) $(LFLAGS) -o isogame
