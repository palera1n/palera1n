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

static const char* get_timestamp(void) {
    static char buffer[32];
    time_t raw_time = time(NULL);
    struct tm *time_info = localtime(&raw_time);
    
    if (time_info) {
        // YYYY-MM-DD HH:MM:SS
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
    } else {
        snprintf(buffer, sizeof(buffer), "0000-00-00 00:00:00");
    }
    return buffer;
}

void log_write(log_level_t level, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    char tag_buf[32];
    snprintf(tag_buf, sizeof(tag_buf), "<%s>", level_name(level));

    printf(" %s%-9s%s [%s%s%s] => ", 
           level_color(level), tag_buf, RESET_COLOR, 
           GRAY_COLOR, get_timestamp(), RESET_COLOR);

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