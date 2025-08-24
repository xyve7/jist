#include <log.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

extern const char *app_name;

void log_base(const char *level, const char *format, va_list args) {
    fprintf(stderr, "[%s] %s: ", app_name, level);
    vfprintf(stderr, format, args);
    fputc('\n', stderr);
}

void log_info(const char *format, ...) {
    va_list args;
    va_start(args, format);

    log_base("INFO", format, args);

    va_end(args);
}
void log_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);

    log_base("WARN", format, args);

    va_end(args);
}
void log_fatal(const char *format, ...) {
    va_list args;
    va_start(args, format);

    log_base("FATAL", format, args);

    va_end(args);

    exit(1);
}
