CPPC=g++
CFLAGS= -pedantic -W -pthread -lpthread
STD=-std=c++11
VERBOSE=-v

all:
	$(CPPC) $(STD) $(CFLAGS) *.cpp -o proxyHttp

clean:
	rm -rf proxyHttp *.o *.dSYM



