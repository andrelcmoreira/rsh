#ifndef COMMON_H_
#define COMMON_H_

#define rsh_log(...) fprintf(stdout, __VA_ARGS__)
#define rsh_error(...) fprintf(stderr, "[-] "__VA_ARGS__)
#define rsh_info(...) fprintf(stdout, "[+] "__VA_ARGS__)

#endif  // !COMMON_H_
