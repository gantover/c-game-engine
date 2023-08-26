CC=clang
CFLAGS=-Wall -g
frameworks=-framework Cocoa -framework OpenGL -framework IOKit -framework GLUT

all: app

app: main.c
	$(CC) $(CFLAGS) -o $@ $^ $(frameworks)
