CC=gcc
CFLAGS=-g -ansi -pedantic-errors -Wall

default: all

all: cserver cclient

cserver: cserver.o
	$(CC) -o cserver cserver.o

cserver.o: cserver.c ccommon.h
	$(CC) $(CFLAGS) -c cserver.c
	
cclient: cclient.o
	$(CC) -o cclient cclient.o

cclient.o: cclient.c ccommon.h
	$(CC) $(CFLAGS) -c cclient.c
 
.PHONY: clean

clean:
	rm -rf cserver cserver.o cclient cclient.o
