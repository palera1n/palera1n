#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PALERAIN_VERSION "3.0.0"

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[31m"
#define GREEN_COLOR   "\033[32m"
#define YELLOW_COLOR  "\033[33m"
#define BLUE_COLOR    "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR    "\033[36m"
#define WHITE_COLOR   "\033[37m"
#define GRAY_COLOR    "\033[90m"

typedef enum {
    LOG_INFO,
    LOG_SUCCESS,
    LOG_WARN,
    LOG_ERROR,
    LOG_VERBOSE
} log_level_t;

#define LOG(fmt, ...)    log_write(LOG_INFO, fmt, ##__VA_ARGS__)
#define SUCCESS(fmt, ...) log_write(LOG_SUCCESS, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...)    log_write(LOG_WARN, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...)   log_write(LOG_ERROR, fmt, ##__VA_ARGS__)
#define VERBOSE(fmt, ...) log_write(LOG_VERBOSE, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

void log_write(log_level_t level, const char *fmt, ...);
void sleep_ms(unsigned ms);

#ifdef __cplusplus
}
#endif

static inline const char* time_now() {
    static char buf[16];
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    strftime(buf, sizeof(buf), "%H:%M:%S", tm_info);
    return buf;
}

extern uint64_t palerain_flags;

#endif // CONSTANTS_H
