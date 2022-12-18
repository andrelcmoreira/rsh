#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

static void usage(char *progname) {
  const char *banner =
    "          _       \n"
    " _ __ ___| |__    \n"
    "| '__/ __| '_ \\  \n"
    "| |  \\__ \\ | | |\n"
    "|_|  |___/_| |_|  \n"
    "(r)everse(sh)ell\n";

  fprintf(stdout, "%s\nusage: %s -p <server_port>\n", banner, progname);
}

static int parse_args(int argc, char *argv[], in_port_t *server_port) {
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

static void read_cli_buffer(int client_fd) {
  char cli_buffer;

  while (read(client_fd, &cli_buffer, sizeof(cli_buffer)) > 0) {
    fprintf(stdout, "%c", cli_buffer);
    cli_buffer = '\0';
  }
}

static void handle_client(int client_fd) {
  char kb_buffer[1024];

  // read the prompt
  read_cli_buffer(client_fd);

  while (1) {
    memset(kb_buffer, 0, sizeof(kb_buffer));
    fgets(kb_buffer, sizeof(kb_buffer), stdin);

    // issue the command
    write(client_fd, kb_buffer, strlen(kb_buffer));
    if (!strcmp(kb_buffer, "exit\n"))
      break;

    // read the result
    read_cli_buffer(client_fd);
  }
}

static int run(struct sockaddr_in *s_addr) {
  int s_fd, c_fd;
  struct sockaddr_in c_addr;
  socklen_t cli_len = sizeof(c_addr);
  struct timeval rd_timeout;

  if ((s_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "[-] fail to create the server socket.\n");

    return EXIT_FAILURE;
  }

  s_addr->sin_family = AF_INET;
  s_addr->sin_addr.s_addr = htonl(INADDR_ANY);

  rd_timeout.tv_sec = 0;
  rd_timeout.tv_usec = 200000;

  // set the timeout for read operations
  setsockopt(s_fd, SOL_SOCKET, SO_RCVTIMEO, &rd_timeout,
             sizeof(struct timeval));

  // bind the server to specified port
  if (bind(s_fd, (struct sockaddr *)s_addr, sizeof(struct sockaddr)) == -1) {
    fprintf(stderr, "[-] fail to bind the server to specified port\n");
    close(s_fd);

    return EXIT_FAILURE;
  }

  if (listen(s_fd, 1) == -1) {
    fprintf(stderr, "[-] fail to configure the server to listen connections\n");
    close(s_fd);

    return EXIT_FAILURE;
  }

  fprintf(stdout, "[+] starting server...\n");

  while (1) {
    c_fd = accept(s_fd, (struct sockaddr *)&c_addr, &cli_len);

    if (c_fd > 0) {
      fprintf(stdout, "[+] client %s connected\n", inet_ntoa(c_addr.sin_addr));
      handle_client(c_fd);
      fprintf(stdout, "[+] client %s disconnected\n",
              inet_ntoa(c_addr.sin_addr));
    }
  }

  close(s_fd);

  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  struct sockaddr_in server_addr;
  int ret;

  memset(&server_addr, 0, sizeof(struct sockaddr_in));

  // parse the server port
  if (parse_args(argc, argv, &server_addr.sin_port)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  ret = run(&server_addr);

  exit(ret);
}
