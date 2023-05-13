#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "rsh.h"

static void usage(const char *progname) {
  const char *banner = BANNER;

  RSH_LOG(
      "%s\nusage: %s [OPTIONS]\n\n"
      "OPTIONS\n"
      " -p <port> Specify the port to bind the server to\n"
      " -h        Show the usage\n", banner, progname);
}

static int parse_args(int argc, char *argv[], struct rsh_ctx_t *ctx) {
  const char *short_opts = "p:h";
  int opt;

  while ((opt = getopt(argc, argv, short_opts)) != -1) {
    switch (opt) {
    case 'p':
      ctx->port = htons(atoi(optarg));
      break;
    case 'h':
      return 1;
    }
  }

  if (!ctx->port) {
    return 1;
  }

  return 0;
}

static void read_cli_buffer(int client_fd) {
  char cli_buffer;

  while (read(client_fd, &cli_buffer, sizeof(cli_buffer)) > 0) {
    RSH_LOG("%c", cli_buffer);
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
    if (!strcmp(kb_buffer, "exit\n")) {
      break;
    }

    // read the result
    read_cli_buffer(client_fd);
  }
}

static int run(struct rsh_ctx_t *ctx) {
  int s_fd, c_fd;
  struct sockaddr_in c_addr;
  socklen_t cli_len = sizeof(c_addr);
  struct timeval rd_timeout;
  struct sockaddr_in addr;

  if ((s_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    RSH_ERROR("fail to create the server socket!\n");

    return 1;
  }

  rd_timeout.tv_sec = 0;
  rd_timeout.tv_usec = 200000;

  // set the timeout for read operations
  setsockopt(s_fd, SOL_SOCKET, SO_RCVTIMEO, &rd_timeout,
             sizeof(struct timeval));

  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = ctx->port;

  // bind the server to specified port
  if (bind(s_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
    RSH_ERROR("fail to bind the server to specified port!\n");
    close(s_fd);

    return 1;
  }

  if (listen(s_fd, 1) == -1) {
    RSH_ERROR("fail to configure the server to listen connections!\n");
    close(s_fd);

    return 1;
  }

  RSH_INFO("starting server...\n");

  while (1) {
    c_fd = accept(s_fd, (struct sockaddr *)&c_addr, &cli_len);

    if (c_fd > 0) {
      RSH_INFO("client %s connected\n", inet_ntoa(c_addr.sin_addr));
      handle_client(c_fd);
      RSH_INFO("client %s disconnected\n", inet_ntoa(c_addr.sin_addr));
    }
  }

  close(s_fd);

  return 0;
}

int main(int argc, char *argv[]) {
  struct rsh_ctx_t ctx;

  memset(&ctx, 0, sizeof(struct rsh_ctx_t));

  // parse the server port
  if (parse_args(argc, argv, &ctx)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  int ret = run(&ctx);

  exit(!ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
