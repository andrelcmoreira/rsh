#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "rsh.h"

#define END_OF_TEXT_BYTE               '\x03'
#define END_OF_TRANSMISSION_BYTE       '\x04'
#define CMD_SEPARATOR                  " ; "

volatile bool user_abort = false;

static void sig_handler(int signum) {
  (void)signum;

  user_abort = true;
}

static void usage(const char *progname) {
  RSH_RAW_LOG(
    "%sv%s\n%s\nUsage: %s [OPTIONS]\n\n"
    "OPTIONS\n"
    " -p <port> Specify the port to bind the server\n"
    " -h        Show this message\n", BANNER, VERSION, FOOTER, progname);
}

static int parse_args(int argc, char *argv[], rsh_cfg_t *restrict cfg) {
  const char *short_opts = "p:h";
  int opt;

  while ((opt = getopt(argc, argv, short_opts)) != -1) {
    switch (opt) {
    case 'p':
      cfg->port = htons(atoi(optarg));
      break;
    case 'h':
      return 1;
    }
  }

  if (!cfg->port) {
    return 1;
  }

  return 0;
}

static void read_cli_buffer(int client_fd) {
  bool eotrs = false;
  bool eotxt = false;
  char cli_buffer;

  while (1) {
    if (read(client_fd, &cli_buffer, sizeof(cli_buffer)) <= 0) {
      break;
    }

    switch (cli_buffer) {
    case END_OF_TRANSMISSION_BYTE:
      eotrs = true;
      break;
    case END_OF_TEXT_BYTE:
      eotxt = true;
      break;
    case ' ':
      RSH_RAW_LOG("%c", cli_buffer);
      if (eotrs && eotxt) {
        return;
      }
      break;
    default:
      RSH_RAW_LOG("%c", cli_buffer);
    }
  }
}

inline static void assemble_cmd(char *kb_cmd, char *user_cmd,
                                size_t *cmd_len) {
  if (kb_cmd[0] != '\n') {
    memcpy(user_cmd, kb_cmd, *cmd_len);
    sprintf(&user_cmd[*cmd_len - 1], "%sprintf \"%c%c\"\n", CMD_SEPARATOR,
            END_OF_TEXT_BYTE, END_OF_TRANSMISSION_BYTE);
  } else {
    sprintf(&user_cmd[*cmd_len - 1], "printf \"%c%c\"\n", END_OF_TEXT_BYTE,
            END_OF_TRANSMISSION_BYTE);
  }

  *cmd_len = strlen(user_cmd);
}

static void handle_client(int client_fd) {
  char user_cmd[1024];
  char client_cmd[1024];
  size_t cmd_len;

  while (!user_abort) {
    read_cli_buffer(client_fd);

    memset(client_cmd, 0, sizeof(client_cmd));
    memset(user_cmd, 0, sizeof(user_cmd));
    fgets(user_cmd, sizeof(user_cmd), stdin);

    cmd_len = strlen(user_cmd);
    assemble_cmd(user_cmd, client_cmd, &cmd_len);
    // issue the command
    write(client_fd, client_cmd, cmd_len);
    if (!strncmp(user_cmd, "exit\n", 5)) {
      break;
    }
  }
}

static int run(const rsh_cfg_t *restrict cfg) {
  int s_fd, c_fd;
  struct sockaddr_in c_addr;
  socklen_t cli_len = sizeof(c_addr);
  struct timeval rd_timeout;
  struct sockaddr_in addr;
  fd_set set;
  int ret;

  if ((s_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    RSH_FATAL("Fail to create the server socket!\n");
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
  addr.sin_port = cfg->port;

  // bind the server to specified port
  if (bind(s_fd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) == -1) {
    RSH_FATAL("Fail to bind the server to specified port!\n");
    close(s_fd);

    return 1;
  }

  if (listen(s_fd, 1) == -1) {
    RSH_FATAL("Fail to configure the server to listen connections!\n");
    close(s_fd);

    return 1;
  }

  RSH_LOG("Starting server...\n");

  while (1) {
    FD_ZERO(&set);
    FD_SET(s_fd, &set);

    ret = select(s_fd + 1, &set, NULL, NULL, NULL);
    if (ret == -1) {
      RSH_FATAL("%s\n", strerror(errno));
      break;
    }

    if (FD_ISSET(s_fd, &set)) {
      c_fd = accept(s_fd, (struct sockaddr *)&c_addr, &cli_len);

      if (c_fd > 0) {
        RSH_SUCCESS("Client %s connected\n", inet_ntoa(c_addr.sin_addr));
        handle_client(c_fd);
        RSH_SUCCESS("Client %s disconnected\n", inet_ntoa(c_addr.sin_addr));
      }
    }
  }

  RSH_LOG("Exiting...\n");

  close(s_fd);

  return 0;
}

int main(int argc, char *argv[]) {
  rsh_cfg_t cfg;

  memset(&cfg, 0, sizeof(rsh_cfg_t));

  // parse the server port
  if (parse_args(argc, argv, &cfg)) {
    usage(argv[0]);
    exit(EXIT_FAILURE);
  }

  signal(SIGINT, sig_handler);
  signal(SIGKILL, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGQUIT, sig_handler);

  int ret = run(&cfg);

  exit(ret);
}
