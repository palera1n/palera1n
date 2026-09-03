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

#ifndef LIBOPENRA1N__DFU_H
#define LIBOPENRA1N__DFU_H

#include <stdint.h>
#include <stdbool.h>

#define DFU_DNLOAD 1
#define DFU_UPLOAD 2
#define DFU_GET_STATUS 3
#define DFU_CLR_STATUS 4
#define DFU_GETSTATE 5
#define DFU_ABORT 6

#define DFU_FILE_SUFFIX_LEN 16
#define EP0_MAX_PACKET_SIZE 0x40
#define DFU_MAX_TRANSFER_SZ 0x800
#define DFU_STATUS_OK 0
#define DFU_STATE_MANIFEST_SYNC 6
#define DFU_STATE_MANIFEST 7
#define DFU_STATE_MANIFEST_WAIT_RESET 8

#ifdef __cplusplus
extern "C" {
#endif

uint16_t dfu_serial_number_get_cpid(char *serial);
bool dfu_serial_number_is_in_dfu_mode(char *serial);
bool dfu_serial_number_is_pwned(char *serial);
bool dfu_serial_number_is_in_yolo_dfu(char *serial);
bool device_serial_number_is_in_pongo_os(char *serial);

#ifdef __cplusplus
}
#endif

#endif // LIBOPENRA1N__DFU_H
