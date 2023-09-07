CC=clang
CFLAGS=-Wall -g
frameworks=-framework Cocoa -framework OpenGL -framework IOKit -framework GLUT

all: app libvector.o

libvector.o: ./lib/libvector.c ./lib/vector.h
	$(CC) $(CFLAGS) -c ./lib/libvector.c

app: main.c libvector.o
	$(CC) $(CFLAGS) -o $@ $^ $(frameworks)
