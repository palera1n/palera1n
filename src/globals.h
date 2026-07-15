#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define PALERAIN_VERSION "3.0.0"

extern uint64_t palerain_flags;
extern char boot_args[0x270];

typedef struct {
    const uint8_t *data;
    size_t data_len;
} payload_t;

extern payload_t g_payload_overlay;
extern payload_t g_payload_ramdisk;
extern payload_t g_payload_pongo;
extern payload_t g_payload_kpf;

#ifdef __cplusplus
extern "C" {
#endif

bool override_payload_from_file(const char *path, payload_t *out);

#ifdef __cplusplus
}
#endif

#endif // GLOBALS_H
