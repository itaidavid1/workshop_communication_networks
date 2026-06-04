CC ?= cc
CFLAGS ?= -O3 -std=c11 -Wall -Wextra -Wpedantic -Werror

BINARIES = server client

all: $(BINARIES)

server: server.o common.o
	$(CC) $(CFLAGS) -o $@ $^

client: client.o common.o
	$(CC) $(CFLAGS) -o $@ $^

server.o: server.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

client.o: client.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

common.o: common.c common.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(BINARIES)

.PHONY: all clean