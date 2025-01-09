#ifndef RSH_H_
#define RSH_H_

#include <stdint.h>

#define BANNER \
    "             _       \n" \
    "    _ __ ___| |__    \n" \
    "   | '__/ __| '_ \\  \n" \
    "   | |  \\__ \\ | | |\n" \
    "   |_|  |___/_| |_| "

#define FOOTER \
    "   (r)everse(sh)ell\n"

// logging
#define RSH_RAW_LOG(...) fprintf(stdout, __VA_ARGS__)
#ifdef LOGGING
#define RSH_LOG(...)     fprintf(stdout, "[*] "__VA_ARGS__)
#define RSH_FATAL(...)   fprintf(stderr, "[\033[0;31m-\033[0m] "__VA_ARGS__)
#define RSH_SUCCESS(...) fprintf(stdout, "[\033[0;32m+\033[0m] "__VA_ARGS__)
#else
#define RSH_LOG(...)
#define RSH_FATAL(...)
#define RSH_SUCCESS(...)
#endif  // LOGGING

typedef struct {
  char ip[16];
  uint16_t port;
  uint16_t retry_count;
  uint16_t retry_interval;
} rsh_cfg_t;

#endif  // !RSH_H_
