CXX=gcc
CFLAGS=-Wall -std=c17
exercise1:
	$(CXX) $(CFLAGS) -o exercise1.out exercise1.c 

catcher:
	$(CXX) $(CFLAGS) -o catcher.out catcher.c 

sender:
	$(CXX) $(CFLAGS) -o sender.out sender.c 
all: exercise1 catcher sender
clean:
	rm -f exercise1.out catcher.out sender.out
.PHONY: clean all