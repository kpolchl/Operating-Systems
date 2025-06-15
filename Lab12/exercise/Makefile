CXX=gcc
CFLAGS=-Wall -std=c17
client:
	$(CXX) $(CFLAGS) -o client.out client.c 
server:
	$(CXX) $(CFLAGS) -o server.out server.c  
all: client server
clean:
	rm -f client.out server.out  
.PHONY: clean all