CPPC=g++
CC=gcc
CFLAGS=-c -g -Wall -pthread -lpthread
STD=-std=c++11
VERBOSE=-v

all: main.o server.o
	$(CPPC) server.o main.o -o servidorHttp -lpthread

clean:
	rm -rf servidorHttp *.o *.dSYM

main.o: main.cc
	$(CPPC) $(CFLAGS) $(STD) main.cc

server.o: server.cc
	$(CPPC) $(CFLAGS) $(STD) server.cc 


