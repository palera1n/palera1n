#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#define A8      0x7000
#define A8X     0x7001
#define S1      0x7002
#define A9      0x8000
#define A9X     0x8001
#define A9_TSMC 0x8003
#define A10     0x8010
#define A10X    0x8011
#define T2      0x8012
#define A11     0x8015

struct DeviceConfiguration {
    uint16_t cpid;
    size_t
    config_hole,
    config_large_leak,
    config_overwrite_pad;
};

struct PayloadConfiguration {
    uint64_t
    tlbi,
    nop_gadget,
    ret_gadget,
    patch_addr,
    ttbr0_addr,
    func_gadget,
    write_ttbr0,
    memcpy_addr,
    aes_crypto_cmd,
    boot_tramp_end,
    ttbr0_vrom_off,
    ttbr0_sram_off,
    gUSBSerialNumber, // ??
    dfu_handle_request,
    usb_core_do_transfer,
    dfu_handle_bus_reset,
    insecure_memory_base,
    handle_interface_request,
    usb_create_string_descriptor,
    usb_serial_number_string_descriptor;
    uint32_t payload_dest_armv7;
};

typedef struct {
    uint16_t cpid;
    const uint8_t *data;
    size_t len;
} yolo_payload_t;

bool checkm8_find_device_configuration_for_cpid(uint16_t cpid, struct DeviceConfiguration *config);
bool checkm8_find_payload_configuration_for_cpid(uint16_t cpid, struct PayloadConfiguration *config);
bool create_pongo_payload_for_device(uint16_t cpid, const uint8_t **payload, size_t *payloadSize);

#endif // PAYLOAD_H
