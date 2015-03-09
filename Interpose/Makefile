PROGS=server
CFLAGS+=-Wall
INCPATH=../include

all: mylib.so $(PROGS)

mylib.o: mylib.c
	gcc -Wall -fPIC -DPIC -L../lib -I$(INCPATH) -c mylib.c mystub.c

mylib.so: mylib.o 
	ld -shared -L../lib -o mylib.so mylib.o mystub.o -ldl

server: server.c
	gcc -Wall -fPIC -DPIC -L../lib -I$(INCPATH) -pthread -o server server.c mystub.c ../lib/libdirtree.so

clean:
	rm -f *.o *.so $(PROGS)
