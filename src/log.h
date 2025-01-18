#include <stdarg.h>
#include <stdio.h>

#define LOG log_to_file

// #define LOG(...)                                                               \
//     fprintf(lf, __VA_ARGS__);                                                  \
//     fprintf(lf, "\n");                                                         \
//     fflush(lf)

void init_log();
void log_to_file(const char* fmt, ...);
