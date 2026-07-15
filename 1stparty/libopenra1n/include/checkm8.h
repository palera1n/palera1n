#ifndef LIBOPENRA1N__CHECKM8_H
#define LIBOPENRA1N__CHECKM8_H

#include <stdbool.h>

#include "payload.h"
#include "shim.h"

#define EP0_MAX_PACKET_SZ (0x40)
#define USB_MAX_STRING_DESCRIPTOR_IDX (10)

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

typedef enum {
    CHECKM8_ERR_NONE = 0,
    CHECKM8_ERR_UNSUPPORTED = 1,
    CHECKM8_ERR_TRANSFER_FAILED = 3,
    CHECKM8_ERR_TIMEOUT = 4,
    CHECKM8_ERR_ABORT = 5,
    CHECKM8_ERR_INVALID_PAYLOAD = 15,

    CHECKM8_ERR_UNDEFINED = 255,
} checkm8_err_t;

#ifdef __cplusplus
extern "C" {
#endif

char *get_usb_serial_number(usb_handle_t *handle);
checkm8_err_t checkm8_stage_reset(const usb_handle_t *handle);
checkm8_err_t checkm8_stage_setup(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig);
checkm8_err_t checkm8_stage_spray(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig);
checkm8_err_t checkm8_stage_patch(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig);
bool prepare_pongo(uint8_t **out, size_t *out_len, const uint8_t *pongo_bin, size_t pongo_bin_len);
checkm8_err_t checkm8_boot_pongo(usb_handle_t *handle, const uint8_t *pongo_bin, size_t pongo_bin_len);

#ifdef __cplusplus
}
#endif

#endif // LIBOPENRA1N__CHECKM8_H
