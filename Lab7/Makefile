CXX=gcc
CFLAGS=-Wall -std=c17
client:
	$(CXX) $(CFLAGS) -o client.out client.c -lrt -lpthread

server:
	$(CXX) $(CFLAGS) -o server.out  server.c -lrt -lpthread

all: client server
clean:
	rm -f server.out client.out 
.PHONY: clean all