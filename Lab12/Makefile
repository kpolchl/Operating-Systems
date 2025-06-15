CXX=gcc
CFLAGS=-Wall -std=c17
ex1_client:
	$(CXX) $(CFLAGS) -o ex1_client.out ex1_client.c 
ex1_server:
	$(CXX) $(CFLAGS) -o ex1_server.out ex1_server.c  
all: ex1_client ex1_server
clean:
	rm -f ex1_client.out ex1_server.out  
.PHONY: clean all