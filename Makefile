CPPC=g++
CFLAGS= -pedantic -W -Wall -pthread -lpthread
STD=-std=c++11
VERBOSE=-v

all:
	$(CPPC) $(STD) $(CFLAGS) *.cpp -o servidorHttp

clean:
	rm -rf servidorHttp *.o *.dSYM



