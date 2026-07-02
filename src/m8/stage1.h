#ifndef STAGE1_H
#define STAGE1_H

#include <stdint.h>
#include <stdbool.h>

#include "payload.h"

typedef struct
{
    uint32_t endpoint, pad_0;
    uint64_t io_buffer;
    uint32_t status, io_len, ret_cnt, pad_1;
    uint64_t callback, next;
} dfu_callback_t;

typedef struct
{
    dfu_callback_t callback;
} checkm8_overwrite_t;

bool generate_stage1(
    void** outbuf,
    size_t* outlen,
    void* payload,
    size_t payload_sz,
    struct DeviceConfiguration *deviceConfig,
    struct PayloadConfiguration *payloadConfig
);

#endif // STAGE1_H
