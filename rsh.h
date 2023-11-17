#ifndef RSH_H_
#define RSH_H_

#include <stdint.h>

#define BANNER \
    "             _       \n" \
    "    _ __ ___| |__    \n" \
    "   | '__/ __| '_ \\  \n" \
    "   | |  \\__ \\ | | |\n" \
    "   |_|  |___/_| |_| "

#define DESC \
    "   (r)everse(sh)ell\n"

// logging
#define RSH_LOG(...)   fprintf(stdout, __VA_ARGS__)
#define RSH_ERROR(...) fprintf(stderr, "[-] "__VA_ARGS__)
#define RSH_INFO(...)  fprintf(stdout, "[+] "__VA_ARGS__)

typedef struct {
  char ip[16];
  uint32_t port;
} rsh_ctx_t;

#endif  // !RSH_H_
