#ifndef BACKEND_SHIM_H
#define BACKEND_SHIM_H

#ifdef __APPLE__
# include "iokit/usb_iokit.h"
#else
# include "libusb/usb_libusb.h"
#endif

bool send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, transfer_ret_t *transfer_ret);
bool send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret);
char *get_usb_serial_number(usb_handle_t *handle);

#endif // BACKEND_SHIM_H
