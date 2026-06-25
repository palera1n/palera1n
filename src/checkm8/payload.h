#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

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
};

bool checkm8_find_device_configuration_for_cpid(int cpid, struct DeviceConfiguration *config);
bool checkm8_find_payload_configuration_for_cpid(int cpid, struct PayloadConfiguration *config);
void create_pongo_payload_for_device(uint16_t cpid, uint8_t **payload, size_t *payloadSize);

#endif // PAYLOAD_H
