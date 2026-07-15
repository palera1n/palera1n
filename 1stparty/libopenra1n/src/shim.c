#include <shim.h>

#include <stdlib.h>
#include <string.h>

#ifdef __APPLE__
# include <usb_iokit.h>
#else
# include <usb_libusb.h>
#endif

transfer_ret_t send_usb_control_request_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len) {
    transfer_ret_t result;
    void *p_data;

    if(w_len == 0) {
        return send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, NULL, 0);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        result = send_usb_control_request(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len);
        free(p_data);
    }

    return result;
}

transfer_ret_t send_usb_control_request_async_no_data(const usb_handle_t *handle, uint8_t bm_request_type, uint8_t b_request, uint16_t w_value, uint16_t w_index, size_t w_len, unsigned usb_abort_timeout) {
    transfer_ret_t result;
    void *p_data;

    if(w_len == 0) {
        return send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, NULL, 0, usb_abort_timeout);
    } else if((p_data = malloc(w_len)) != NULL) {
        memset(p_data, '\0', w_len);
        result = send_usb_control_request_async(handle, bm_request_type, b_request, w_value, w_index, p_data, w_len, usb_abort_timeout);
        free(p_data);
    }

    return result;
}
