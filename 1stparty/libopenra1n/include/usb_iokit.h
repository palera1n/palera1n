#ifdef __APPLE__

#ifndef LIBOPENRA1N__IOKIT_BACKEND_H
#define LIBOPENRA1N__IOKIT_BACKEND_H

#include <stdbool.h>

#include <IOKit/IOCFPlugIn.h>
#include <IOKit/usb/IOUSBLib.h>

#define USB_TIMEOUT (5)

#if TARGET_OS_IPHONE
# define kUSBPipeStalled kUSBHostReturnPipeStalled
#else
# define kUSBPipeStalled kIOUSBPipeStalled
#endif

#if TARGET_OS_IPHONE
# define kUSBDeviceClassName "IOUSBHostDevice"
#else
# define kUSBDeviceClassName kIOUSBDeviceClassName
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

typedef void (*usb_hotplug_callback_t)(
    io_service_t service,
    bool connected,
    void *user
);

typedef struct {
    uint16_t vid, pid;
    io_service_t serv;
    IOUSBDeviceInterface320 **device;
    IOUSBInterfaceInterface245 **interface;
    CFRunLoopSourceRef async_event_source;
} usb_handle_t;

#ifdef __cplusplus
extern "C" {
#endif

bool query_usb_interface(io_service_t serv, CFUUIDRef plugin_type, CFUUIDRef interface_type, LPVOID *interface);
void close_usb_handle(usb_handle_t *handle);
bool open_usb_device(io_service_t serv, usb_handle_t *handle);
bool wait_usb_handle(usb_handle_t *handle);
void reset_usb_handle(usb_handle_t *handle);
transfer_ret_t send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout);
transfer_ret_t send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_interface_bulk_transfer(const usb_handle_t *handle, void *data, uint32_t len);
void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid);

#ifdef __cplusplus
}
#endif

#endif // LIBOPENRA1N__IOKIT_BACKEND_H

#endif // __APPLE__
