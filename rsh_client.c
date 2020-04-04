#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

static void usage(char *progname)
{
    fprintf(stderr, "usage: %s [-s server_ip] [-p server_port]\n", progname);
}

static int parse_args(int argc, char *argv[], struct in_addr *server_addr, in_port_t *server_port)
{
    int opt;

    while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
        switch (opt) {
        case 'p':
            *server_port = htons(atoi(optarg));
            break;

        case 's':
            inet_aton(optarg, server_addr);
            break;

        case 'h':
            return 1;
        }
    }

    if (!(*server_port) || !server_addr->s_addr)
        return 1;

    return 0;
}

int main(int argc, char *argv[])
{
    int server_fd = 0;
    struct sockaddr_in server_addr;
    char server_buffer[1024];
    char client_buffer[1024];
    struct timeval op_timeout;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    /* Parse the server ip and port. */
    if (parse_args(argc, argv, &server_addr.sin_addr, &server_addr.sin_port)) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "fail to create the client socket.\n");
        exit(EXIT_FAILURE);
    }

    /* Define 5 seconds as timeout for communication with server. */
    op_timeout.tv_sec = 5;
    op_timeout.tv_usec = 0;

    /* Set the timeout for read operations. */
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &op_timeout, sizeof(struct timeval));

    /* Try to connect to server. */
    if (connect(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr))) {
        fprintf(stderr, "fail to connect to server!\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        fprintf(stdout, "rsh$ ");

        memset(client_buffer, 0, sizeof(client_buffer));
        memset(server_buffer, 0, sizeof(server_buffer));

        fgets(client_buffer, sizeof(client_buffer), stdin);

        /* Remove the '\n' char. */
        client_buffer[strlen(client_buffer)] = 0;
        client_buffer[strlen(client_buffer) - 1] = '\0';

        /* Issue the command */
        write(server_fd, client_buffer, strlen(client_buffer));

        if (!strcmp(client_buffer, "exit"))
            break;

        /* Read the reply */
        while (read(server_fd, server_buffer, sizeof(server_buffer)) == sizeof(server_buffer))
            /* Show the command result */
            fprintf(stdout, "%s", server_buffer);
    }

    close(server_fd);
    exit(EXIT_SUCCESS);
}
