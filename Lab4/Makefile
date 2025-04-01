CXX=gcc
CFLAGS=-Wall -std=c17
fork_test:
	$(CXX) $(CFLAGS) -o fork_test.out fork_test.c -g
zad1:
	$(CXX) $(CFLAGS) -o zad1.out zad1.c -g
zad2:
	$(CXX) $(CFLAGS) -o zad2.out zad2.c -g
all: fork_test zad1 zad2
clean:
	rm fork_test.out zad1.out zad2.out
.PHONY: clean all