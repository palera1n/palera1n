#ifndef __APPLE__

#ifndef LIBOPENRA1N__LIBUSB_BACKEND_H
#define LIBOPENRA1N__LIBUSB_BACKEND_H

#include <stdbool.h>
#include <libusb-1.0/libusb.h>

#define USB_TIMEOUT (5)

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
    struct libusb_device_handle *device;
} usb_handle_t;

void close_usb_handle(usb_handle_t *handle);
void reset_usb_handle(const usb_handle_t *handle);
bool wait_usb_handle(usb_handle_t *handle);
void usb_async_cb(struct libusb_transfer *transfer);
transfer_ret_t send_usb_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_usb_control_request_async(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len, unsigned usb_abort_timeout);
transfer_ret_t send_interface_control_request(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, void *p_data, size_t w_len);
transfer_ret_t send_interface_bulk_transfer(const usb_handle_t *handle, void *data, int len);
void init_usb_handle(usb_handle_t *handle, uint16_t vid, uint16_t pid);

#endif // LIBOPENRA1N__LIBUSB_BACKEND_H

#endif // __APPLE__
