CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wpedantic -O3
VERSION = $(shell git describe --tags)

all: client server

client: rsh_client.c
	$(CC) -DVERSION=\"$(VERSION)\" $(CFLAGS) -o rsh_$@ $^

server: rsh_server.c
	$(CC) -DVERSION=\"$(VERSION)\" $(CFLAGS) -o rsh_$@ $^

clean:
	@rm -f rsh_client rsh_server
