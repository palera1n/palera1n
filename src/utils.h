#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PALERAIN_VERSION "3.0.0"

#include <stdint.h>
#include <stdio.h>

#ifdef __WIN32
# include <windows.h>
#else
# include <time.h>
#endif

#define LOG(fmt, ...) \
printf("(%s:%d) => " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

static void sleep_ms(unsigned ms) {
    #ifdef __WIN32
    Sleep(ms);
    #else
    struct timespec ts;

    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
    #endif
}

static uint64_t palerain_flags = 0;

#endif // CONSTANTS_H
