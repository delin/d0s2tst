CC	= gcc
FLAGS	= -Wall
CFLAGS	= -g -O3 -I/usr/include -march=native -msse -msse2 -mmmx -mfpmath=387,sse
LDFLAGS = -L/usr/lib
LIBS	= -lpthread -lm

all: main x32
x64: main
main: main.o
	$(CC) $(LDFLAGS) -o d0s2tst main.o $(LIBS)
main.o: main.c
	$(CC) $(FLAGS) $(CFLAGS) -c main.c

x32: main_i386.o
	$(CC) $(LDFLAGS) -o d0s2tst.i386 main.o $(LIBS) -m32
main_i386.o: main.c
	rm -f *.o
	$(CC) $(FLAGS) $(CFLAGS) -c main.c -m32

clean:
	rm -f *.o *.gch d0s2tst*
