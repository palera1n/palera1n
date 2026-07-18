#include <utils.h>

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>
#ifdef _WIN32
# include <windows.h>
#endif
#include <time.h>

bool gSilentLogs = false;
bool gColoredLogs = true;

static const char *level_names[] = {
    [LOG_INFO]          = "INFO",
    [LOG_ERROR]         = "ERROR",
    [LOG_VERBOSE]       = "VERBOSE",
};

static const char *level_colors[] = {
    [LOG_INFO]          = CYAN_COLOR,
    [LOG_ERROR]         = RED_COLOR,
    [LOG_VERBOSE]       = RESET_COLOR,
};

static inline void _get_timestamp(char *buf, size_t len)
{
    time_t t = time(NULL);
    struct tm tm_info;

    #ifdef _WIN32
    localtime_s(&tm_info, &t);
    #else
    localtime_r(&t, &tm_info);
    #endif

    strftime(buf, len, "%Y/%m/%d %H:%M:%S", &tm_info);
}

void log_write_internal(log_level_t level, const char *fmt, ...)
{
    if (gSilentLogs) return;

    va_list args;
    va_start(args, fmt);

    char tag_buf[32];
    char ts[32];

    _get_timestamp(ts, sizeof(ts));
    snprintf(tag_buf, sizeof(tag_buf), "<%s>", level_names[level]);

    if (!gColoredLogs) {
        printf(" %9s [%s] => ", tag_buf, ts);
    } else {
        printf(" %s%9s%s [%s%s%s] => ",
               level_colors[level],
               tag_buf,
               RESET_COLOR,
               GRAY_COLOR,
               ts,
               RESET_COLOR);
    }

    vprintf(fmt, args);
    putchar('\n');

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
