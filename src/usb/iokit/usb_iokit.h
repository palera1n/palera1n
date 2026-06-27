#ifdef __APPLE__

#ifndef IOKIT_BACKEND_H
#define IOKIT_BACKEND_H

#include <stdbool.h>

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#define USB_TIMEOUT (5)

#ifdef __APPLE__
# if TARGET_OS_IPHONE
#  define kUSBPipeStalled kUSBHostReturnPipeStalled
# else
#  define kUSBPipeStalled kIOUSBPipeStalled
# endif
#endif

#ifdef __APPLE__
# if TARGET_OS_IPHONE
#  define kUSBDeviceClassName "IOUSBHostDevice"
# else
#  define kUSBDeviceClassName kIOUSBDeviceClassName
# endif
#endif

enum usb_transfer {
    USB_TRANSFER_OK,
    USB_TRANSFER_ERROR,
    USB_TRANSFER_STALL,
};

typedef struct {
    enum usb_transfer ret;
    uint32_t sz;
} transfer_ret_t;

typedef struct {
    uint16_t vid, pid;
    io_service_t serv;
    IOUSBDeviceInterface320 **device;
    CFRunLoopSourceRef async_event_source;
} usb_handle_t;

bool query_usb_interface(io_service_t serv, CFUUIDRef plugin_type, CFUUIDRef interface_type, LPVOID *interface);
void close_usb_device(usb_handle_t *handle);
void close_usb_handle(usb_handle_t *handle);
bool open_usb_device(io_service_t serv, usb_handle_t *handle);
bool wait_usb_handle(usb_handle_t *handle);
void reset_usb_handle(usb_handle_t *handle);
bool send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, transfer_ret_t *transfer_ret);
bool send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret);
void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid);

#endif // IOKIT_BACKEND_H

#endif // __APPLE__
