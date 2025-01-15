CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wpedantic -O3 -s
VERSION != git describe --tags
PARAMS = -DVERSION=\"$(VERSION)\"
CLIENT_PARAMS =

all: client server

client: rsh_client.c
	$(CC) $(CLIENT_PARAMS) $(PARAMS) $(CFLAGS) -o rsh_$@ $?

server: rsh_server.c
	$(CC) $(PARAMS) $(CFLAGS) -o rsh_$@ $?

clean:
	@rm -f rsh_client rsh_server
