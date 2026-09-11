/*
 * palera1n - https://palera.in
 *
 * Copyright (c) 2026 palera1n team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <payload.h>

#include <stdbool.h>
#include <stdint.h>

#include <utils.h>

#include <gen/payloads/yolo_s8000.h>
#include <gen/payloads/yolo_s8001.h>
#include <gen/payloads/yolo_s8003.h>
#include <gen/payloads/yolo_t7000.h>
#include <gen/payloads/yolo_t7001.h>
#include <gen/payloads/yolo_t8010.h>
#include <gen/payloads/yolo_t8011.h>
#include <gen/payloads/yolo_t8012.h>
#include <gen/payloads/yolo_t8015.h>

bool checkm8_find_device_configuration_for_cpid(
    uint16_t cpid,
    struct DeviceConfiguration *config)
{
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case A11:
        case T2:
        case A10X:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x540;
            config->config_hole = 6;
            break;
        case A10:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x5C0;
            config->config_hole = 5;
            break;
        case A9X:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x5C0;
            config->config_hole = 6;
            break;
        case A9_TSMC:
        case A9:
        case A8X:
        case A8:
            config->cpid = cpid;
            config->config_large_leak = 0;
            config->config_overwrite_pad = 0x500;
            config->config_hole = 0;
            break;
        default:
            LOG_ERROR("CPID 0x%x is not supported!", cpid);
            foundMatchingConfiguration = false;
            break;
    }
    return foundMatchingConfiguration;
}

bool checkm8_find_payload_configuration_for_cpid(
    uint16_t cpid,
    struct PayloadConfiguration *config)
{
    bool foundMatchingConfiguration = true;
    switch (cpid) {
        case A11:
            config->insecure_memory_base                = 0x18001C000;
            config->func_gadget                         = 0x10000a998;
            config->tlbi                                = 0x1000004AC;
            config->write_prim                          = 0x100009c48;
            config->arm_clean_invalidate_dcache_line    = 0x1000004e4;
            config->arm_invalidate_icache               = 0x1000004c0;
            config->enter_critical_section              = 0x10000f958;
            config->exit_critical_section               = 0x10000f9a0;
            config->write_ttbr0                         = 0x10000045c;
            config->tlbi                                = 0x1000004ac;
            config->TTBR0_PATCH_BASE                    = 0x180020000;
            config->TTBR0_BASE                          = 0xc000;
            config->bootstrap_task_lr                   = 0x180015f88;
            config->payload_start_offset                = 0x800;
            break;
        case T2:
            config->insecure_memory_base                = 0x18001C000;
            config->func_gadget                         = 0x100008d8c;
            config->write_prim                          = 0x100008058;
            config->arm_clean_invalidate_dcache_line    = 0x1000004cc;
            config->arm_invalidate_icache               = 0x1000004a8;
            config->enter_critical_section              = 0x10000f9b8;
            config->exit_critical_section               = 0x10000fa00;
            config->write_ttbr0                         = 0x100000444;
            config->tlbi                                = 0x100000494;
            config->TTBR0_PATCH_BASE                    = 0x180020000;
            config->TTBR0_BASE                          = 0xc000;
            config->bootstrap_task_lr                   = 0x180015f78;
            config->payload_start_offset                = 0x800;
            break;
        case A10X:
            config->insecure_memory_base                = 0x1800B0000;
            config->func_gadget                         = 0x10000cce4;
            config->write_prim2                         = 0x100001804;
            config->arm_clean_invalidate_dcache_line    = 0x10000047c;
            config->arm_invalidate_icache               = 0x100000458;
            config->enter_critical_section              = 0x10000a658;
            config->exit_critical_section               = 0x10000a6a0;
            config->write_ttbr0                         = 0x1000003F4;
            config->tlbi                                = 0x100000444;
            config->TTBR0_PATCH_BASE                    = 0x1800b4000;
            config->TTBR0_BASE                          = 0xa0000;
            config->bootstrap_task_lr                   = 0x1800a9f88;
            config->payload_start_offset                = 0x800;
            break;
        case A10:
            config->insecure_memory_base                = 0x1800B0000;
            config->func_gadget                         = 0x10000cc44;
            config->write_prim2                         = 0x100001808;
            config->arm_clean_invalidate_dcache_line    = 0x10000046c;
            config->arm_invalidate_icache               = 0x100000448;
            config->enter_critical_section              = 0x10000A4B8;
            config->exit_critical_section               = 0x10000A514;
            config->write_ttbr0                         = 0x1000003E4;
            config->tlbi                                = 0x100000434;
            config->TTBR0_PATCH_BASE                    = 0x1800b4000;
            config->TTBR0_BASE                          = 0xa0000;
            config->bootstrap_task_lr                   = 0x1800a9f68;
            config->payload_start_offset                = 0x800;
            break;
        case A9X:
            config->insecure_memory_base                = 0x180000000;
            config->func_gadget                         = 0x10000cd38;
            config->write_prim2                         = 0x100001a78;
            config->arm_clean_invalidate_dcache_line    = 0x10000043c;
            config->arm_invalidate_icache               = 0x100000418;
            config->enter_critical_section              = 0x100009b24;
            config->exit_critical_section               = 0x100009b88;
            config->write_ttbr0                         = 0x1000003b4;
            config->tlbi                                = 0x100000404;
            config->TTBR0_PATCH_BASE                    = 0x180004000;
            config->TTBR0_BASE                          = 0x50000;
            config->bootstrap_task_lr                   = 0x180059f58;
            config->payload_start_offset                = 0x800;
            break;
        case A9:
            config->insecure_memory_base                = 0x180380000;
            config->func_gadget                         = 0x10000de0c;
            config->write_prim2                         = 0x100001bc0;
            config->arm_clean_invalidate_dcache_line    = 0x10000042c;
            config->arm_invalidate_icache               = 0x100000408;
            config->bootstrap_task_lr                   = 0x1800c2f58;
            config->payload_start_offset                = 0xc0;
            break;
        case A9_TSMC:
            config->insecure_memory_base                = 0x180380000;
            config->func_gadget                         = 0x10000de0c;
            config->write_prim2                         = 0x100001bc0;
            config->arm_clean_invalidate_dcache_line    = 0x10000042c;
            config->arm_invalidate_icache               = 0x100000408;
            config->bootstrap_task_lr                   = 0x1800c2f58;
            config->payload_start_offset                = 0xc0;
            break;
        case A8X:
            config->insecure_memory_base                = 0x180380000;
            config->func_gadget                         = 0x100010df4;
            config->write_prim                          = 0x10000ed5c;
            config->arm_clean_invalidate_dcache_line    = 0x100000448;
            config->arm_invalidate_icache               = 0x100000424;
            config->bootstrap_task_lr                   = 0x1800c2f68;
            config->payload_start_offset                = 0xc0;
            break;
        case A8:
            config->insecure_memory_base                = 0x180380000;
            config->func_gadget                         = 0x10000ddf4;
            config->write_prim                          = 0x10000bc2c;
            config->arm_clean_invalidate_dcache_line    = 0x100000448;
            config->arm_invalidate_icache               = 0x100000424;
            config->bootstrap_task_lr                   = 0x1800c2f68;
            config->payload_start_offset                = 0xc0;
            break;
        default:
            LOG_ERROR("CPID 0x%x is not supported!", cpid);
            foundMatchingConfiguration = false;
    }
    return foundMatchingConfiguration;
}

static const yolo_payload_t yolo_payloads[] = {
    {A11, payloads_yolo_t8015_bin, payloads_yolo_t8015_bin_len},
    {T2, payloads_yolo_t8012_bin, payloads_yolo_t8012_bin_len},
    {A10X, payloads_yolo_t8011_bin, payloads_yolo_t8011_bin_len},
    {A10, payloads_yolo_t8010_bin, payloads_yolo_t8010_bin_len},
    {A9_TSMC, payloads_yolo_s8003_bin, payloads_yolo_s8003_bin_len},
    {A9X, payloads_yolo_s8001_bin, payloads_yolo_s8001_bin_len},
    {A9, payloads_yolo_s8000_bin, payloads_yolo_s8000_bin_len},
    {A8X, payloads_yolo_t7001_bin, payloads_yolo_t7001_bin_len},
    {A8, payloads_yolo_t7000_bin, payloads_yolo_t7000_bin_len},
};

bool create_pongo_payload_for_device(
    uint16_t cpid,
    const uint8_t **payload,
    size_t *payloadSize)
{
    LOG_DEBUG("Preparing YoloDFU payload for CPID 0x%x", cpid);

    *payload = NULL;
    *payloadSize = 0;

    size_t count = sizeof(yolo_payloads) / sizeof(yolo_payloads[0]);

    for (size_t i = 0; i < count; i++) {
        if (yolo_payloads[i].cpid == cpid) {
            *payload = yolo_payloads[i].data;
            *payloadSize = yolo_payloads[i].len;
            return true;
        }
    }

    LOG("Failed to prepare payload for device with CPID 0x%x", cpid);
    return false;
}
