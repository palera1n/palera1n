#include "usb_shim.h"

#ifdef __APPLE__
# include "usb_iokit.h"
#else
# include "usb_libusb.h"
#endif

bool send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, transfer_ret_t *transfer_ret) {
    bool ret = false;
    void *p_data;

    if(w_len == 0) {
        ret = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, transfer_ret);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        ret = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, transfer_ret);
        free(p_data);
    }
    return ret;
}

bool send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout, transfer_ret_t *transfer_ret) {
    bool ret = false;
    void *p_data;

    if(w_len == 0) {
        ret = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, usb_abort_timeout, transfer_ret);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        ret = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, usb_abort_timeout, transfer_ret);
        free(p_data);
    }
    return ret;
}
