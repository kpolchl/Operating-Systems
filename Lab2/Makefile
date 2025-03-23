CC = gcc

CFLAGS = -Wall -O0

CLIENT = client
CLIENT_STATIC = client-static.out
CLIENT_SHARED = client-shared.out
CLIENT_DYNAMIC = client-dynamic.out
SHARED_LIB = libcollatz.so
STATIC_LIB = libcollatz.a

.PHONY: all
all: static shared dynamic

static: $(STATIC_LIB) $(CLIENT_STATIC)

shared: $(SHARED_LIB) $(CLIENT_SHARED)

dynamic: $(SHARED_LIB) $(CLIENT_DYNAMIC)

collatz.o: collatz.c
	$(CC) -c collatz.c -o collatz.o

$(STATIC_LIB): collatz.o
	ar -rcs $(STATIC_LIB) collatz.o
	rm -f collatz.o  

$(SHARED_LIB): collatz.c
	$(CC) -fPIC -c collatz.c -o collatz.o
	$(CC) -shared collatz.o -o $(SHARED_LIB)
	rm -f collatz.o  

$(CLIENT_STATIC): client.c $(STATIC_LIB)
	$(CC) $(CFLAGS) client.c $(STATIC_LIB) -o $(CLIENT_STATIC)

$(CLIENT_SHARED): client.c $(SHARED_LIB)
	$(CC) $(CFLAGS) client.c -L. -lcollatz -Wl,-rpath,. -o $(CLIENT_SHARED)

$(CLIENT_DYNAMIC): client.c $(SHARED_LIB)
	$(CC) $(CFLAGS) -D DYNAMIC_LOAD client.c -o $(CLIENT_DYNAMIC) -ldl

.PHONY: clean
clean:
	rm -f $(CLIENT_STATIC) $(CLIENT_SHARED) $(CLIENT_DYNAMIC) $(STATIC_LIB) $(SHARED_LIB) *.o