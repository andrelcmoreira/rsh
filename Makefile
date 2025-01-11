CC = gcc
CFLAGS = -Wall -Werror -Wextra -Wpedantic -O3 -s
VERSION != git describe --tags
PARAMS = -DVERSION=\"$(VERSION)\"
ifeq ($(LOGGING),ON)
PARAMS += -DLOGGING=ON
endif

# client-specific pre-processing parameters
CLIENT_PARAMS =
ifdef RETRY_COUNT
CLIENT_PARAMS += -DRETRY_COUNT=$(RETRY_COUNT)
endif
ifdef SERVER_PORT
CLIENT_PARAMS += -DSERVER_PORT=$(SERVER_PORT)
endif
ifdef SERVER_ADDRESS
CLIENT_PARAMS += -DSERVER_ADDRESS="$(SERVER_ADDRESS)"
endif
ifdef RETRY_INTERVAL
CLIENT_PARAMS += -DRETRY_INTERVAL=$(RETRY_INTERVAL)
endif

all: client server

client: rsh_client.c
	$(CC) $(CLIENT_PARAMS) $(PARAMS) $(CFLAGS) -o rsh_$@ $?

server: rsh_server.c
	$(CC) $(PARAMS) $(CFLAGS) -o rsh_$@ $?

clean:
	@rm -f rsh_client rsh_server
