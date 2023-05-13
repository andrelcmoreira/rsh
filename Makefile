CC = gcc
CFLAGS = -Wall -Werror -Wextra -O3

all: client server

client: rsh_client.c
	$(CC) $(CFLAGS) -o rsh_$@ $^

server: rsh_server.c
	$(CC) $(CFLAGS) -o rsh_$@ $^

clean:
	-rm rsh_client rsh_server
