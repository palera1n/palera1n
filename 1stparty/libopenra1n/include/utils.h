#ifndef LIBOPENRA1N__UTILS_H
#define LIBOPENRA1N__UTILS_H

#include <stdint.h>
#include <stdbool.h>

#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[31m"
#define GREEN_COLOR   "\033[32m"
#define YELLOW_COLOR  "\033[33m"
#define BLUE_COLOR    "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR    "\033[36m"
#define WHITE_COLOR   "\033[37m"
#define GRAY_COLOR    "\033[90m"

extern uint8_t gDebugLevel;
extern bool gSilentLogs;
extern bool gColoredLogs;

typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_VERBOSE
} log_level_t;

#ifdef __cplusplus
extern "C" {
#endif

void log_write_internal(log_level_t level, const char* func, const char* file, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#define LOG(fmt, ...)           log_write_internal(LOG_INFO, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)     log_write_internal(LOG_ERROR, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)     log_write_internal(LOG_VERBOSE, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void sleep_ms(unsigned ms);

#endif // LIBOPENRA1N__UTILS_H
