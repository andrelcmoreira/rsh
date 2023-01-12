#ifndef COMMON_H_
#define COMMON_H_

#include <stdint.h>

#define BANNER \
    "          _       \n" \
    " _ __ ___| |__    \n" \
    "| '__/ __| '_ \\  \n" \
    "| |  \\__ \\ | | |\n" \
    "|_|  |___/_| |_|  \n" \
    "(r)everse(sh)ell\n"

// logging
#define RSH_LOG(...)   fprintf(stdout, __VA_ARGS__)
#define RSH_ERROR(...) fprintf(stderr, "[-] "__VA_ARGS__)
#define RSH_INFO(...)  fprintf(stdout, "[+] "__VA_ARGS__)

struct rsh_ctx_t {
  char ip[16];
  uint32_t port;
};

#endif  // !COMMON_H_
