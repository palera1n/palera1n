#ifndef LIBOPENRA1N__BACKEND_SHIM_H
#define LIBOPENRA1N__BACKEND_SHIM_H

#ifdef __APPLE__
# include "usb_iokit.h"
#else
# include "usb_libusb.h"
#endif

transfer_ret_t send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len);
transfer_ret_t send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout);

#endif // LIBOPENRA1N__BACKEND_SHIM_H
