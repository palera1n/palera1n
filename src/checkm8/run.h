#ifndef RUN_H
#define RUN_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#ifndef __APPLE__
# include <libusb-1.0/libusb.h>
# include <openssl/evp.h>
# include <stdbool.h>
# include <string.h>
# include <stddef.h>
#else
# include <CommonCrypto/CommonCrypto.h>
# include <CoreFoundation/CoreFoundation.h>
# include <IOKit/IOCFPlugIn.h>
# include <IOKit/usb/IOUSBLib.h>
#endif

#include "dfu.h"

#define APPLE_VID (0x5AC)
#define DFU_MODE_PID (0x1227)
#define MAX_BLOCK_SZ (0x50)
#define EP0_MAX_PACKET_SZ (0x40)
#define USB_MAX_STRING_DESCRIPTOR_IDX (10)

#define USB_TIMEOUT (5)

#ifdef __APPLE__
# if TARGET_OS_IPHONE
#  define kUSBPipeStalled kUSBHostReturnPipeStalled
# else
#  define kUSBPipeStalled kIOUSBPipeStalled
# endif
#endif

#ifndef MIN
# define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifdef __cplusplus
extern "C" {
#endif

enum exploit_stage {
    STAGE_PREPARE = 0,
    STAGE_RESET = 1,
    STAGE_SETUP = 2,
    STAGE_SPRAY = 3,
    STAGE_PATCH = 4,
    STAGE_PONGO = 5,
    STAGE_JAILBREAK = 6,
    STAGE_DONE = 7,
};

enum usb_transfer {
    USB_TRANSFER_OK,
    USB_TRANSFER_ERROR,
    USB_TRANSFER_STALL
};

typedef struct {
    enum usb_transfer ret;
    uint32_t sz;
} transfer_ret_t;

typedef struct {
    uint16_t vid, pid;
    #ifndef __APPLE__
    struct libusb_device_handle *device;
    #else
    io_service_t serv;
    IOUSBDeviceInterface320 **device;
    CFRunLoopSourceRef async_event_source;
    #endif
} usb_handle_t;

typedef bool (*usb_check_cb_t)(usb_handle_t *, void *);

bool gaster_checkm8(usb_handle_t *handle);
int checkm8();
bool checkm9();

#ifdef __cplusplus
}
#endif

#endif // RUN_H
