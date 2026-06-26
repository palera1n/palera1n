#ifndef CONSTANTS_H
#define CONSTANTS_H

#define PALERAIN_VERSION "3.0.0"

#include <stdint.h>
#include <stdio.h>

#define LOG(fmt, ...) \
printf("(%s:%d) => " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

void sleep_ms(unsigned ms);

extern uint64_t palerain_flags;

#endif // CONSTANTS_H
