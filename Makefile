CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wpedantic -O3 -s
VERSION != git describe --tags

all: client server

client: rsh_client.c
	$(CC) -DLOGGING="$(LOGGING)" -DVERSION=\"$(VERSION)\" $(CFLAGS) -o rsh_$@ $?

server: rsh_server.c
	$(CC) -DLOGGING="$(LOGGING)" -DVERSION=\"$(VERSION)\" $(CFLAGS) -o rsh_$@ $?

clean:
	@rm -f rsh_client rsh_server
