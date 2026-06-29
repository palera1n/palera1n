#ifndef CHECKM8_H
#define CHECKM8_H

#include <stdbool.h>

#include "payload.h"

#include "../usb/shim.h"

#define EP0_MAX_PACKET_SZ (0x40)
#define USB_MAX_STRING_DESCRIPTOR_IDX (10)

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

bool checkm8_stage_reset(const usb_handle_t *handle);
bool checkm8_stage_setup(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig);
bool checkm8_stage_spray(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig);
bool checkm8_stage_patch(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig);
bool checkm8_boot_pongo(usb_handle_t *handle);

#endif // CHECKM8_H
