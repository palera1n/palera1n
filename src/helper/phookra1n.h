#ifndef PALERA1N_CHECKRA1N_HELPER_H
#define PALERA1N_CHECKRA1N_HELPER_H

#include <stdint.h>
#include <stdio.h>

#ifdef DEV_BUILD
#define dprintf(...) printf(__VA_ARGS__)
#else
#define dprintf(...)
#endif

extern uint8_t t7000_bin[];
extern uint32_t t7000_bin_len;

extern uint8_t t8011_bin[];
extern uint32_t t8011_bin_len;

extern uint8_t t8012_bin[];
extern uint32_t t8012_bin_len;

#if defined(__x86_64__)
void setup_hooks_x86_64(void *stream);
#elif defined(__arm64__)
void setup_hooks_arm64(void *stream);
#endif

#endif
