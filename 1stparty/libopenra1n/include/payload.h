/*
 * palera1n - https://palera.in
 *
 * Copyright (c) 2026 palera1n team
 *
 * SPDX-License-Identifier: Apache-2.0
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

#ifndef LIBOPENRA1N__PAYLOAD_H
#define LIBOPENRA1N__PAYLOAD_H

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
    insecure_memory_base,
    func_gadget,
    write_prim,
    write_prim2,
    arm_clean_invalidate_dcache_line,
    arm_invalidate_icache,
    enter_critical_section,
    exit_critical_section,
    write_ttbr0,
    tlbi,
    TTBR0_PATCH_BASE,
    TTBR0_BASE,
    bootstrap_task_lr,
    payload_start_offset;
};

typedef struct {
    uint16_t cpid;
    const uint8_t *data;
    size_t len;
} yolo_payload_t;

bool checkm8_find_device_configuration_for_cpid(uint16_t cpid, struct DeviceConfiguration *config);
bool checkm8_find_payload_configuration_for_cpid(uint16_t cpid, struct PayloadConfiguration *config);
bool create_pongo_payload_for_device(uint16_t cpid, const uint8_t **payload, size_t *payloadSize);

#endif // LIBOPENRA1N__PAYLOAD_H
