#include "log.h"

FILE* lf; // Log file

void init_log() { lf = fopen("t4.log", "w+"); }

void log_to_file(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    vfprintf(lf, fmt, args);
    fprintf(lf, "\n");
    fflush(lf);

    va_end(args);
}
