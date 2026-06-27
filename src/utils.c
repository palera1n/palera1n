#include "utils.h"

#ifdef _WIN32
# include <windows.h>
#else
# include <time.h>
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdarg.h>

static const char* level_color(log_level_t level) {
    switch (level) {
        case LOG_INFO:    return CYAN_COLOR;
        case LOG_SUCCESS: return GREEN_COLOR;
        case LOG_WARN:    return YELLOW_COLOR;
        case LOG_ERROR:   return RED_COLOR;
        case LOG_VERBOSE: return GRAY_COLOR;
        default:          return RESET_COLOR;
    }
}

static const char* level_name(log_level_t level) {
    switch (level) {
        case LOG_INFO:    return "INFO";
        case LOG_SUCCESS: return "SUCCESS";
        case LOG_WARN:    return "WARN";
        case LOG_ERROR:   return "ERROR";
        case LOG_VERBOSE: return "VERBOSE";
        default:          return "LOG";
    }
}

void log_write(log_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("%s[%s]%s [%s] => ", level_color(level), level_name(level), RESET_COLOR, time_now());

    vprintf(fmt, args);
    printf("\n");

    va_end(args);
}

void sleep_ms(unsigned ms) {
    #ifdef _WIN32
    Sleep(ms);
    #else
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
    #endif
}

uint64_t palerain_flags = 0;
