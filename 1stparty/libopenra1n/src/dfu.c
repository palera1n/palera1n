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

#include <dfu.h>

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

uint16_t dfu_serial_number_get_cpid(char *serial) {
    char *cpid = strstr(serial, "CPID:");
    if (cpid == NULL) { return -1; }
    return strtol(cpid + 5, NULL, 16);
}

uint32_t dfu_serial_number_get_bdid(char *serial) {
    char *bdid = strstr(serial, "BDID:");
    if (bdid == NULL) { return -1; }
    return strtoul(bdid + 5, NULL, 16);
}

uint64_t dfu_serial_number_get_ecid(char *serial) {
    char *ecid = strstr(serial, "ECID:");
    if (ecid == NULL) { return -1; }
    return strtoull(ecid + 5, NULL, 16);
}

bool dfu_serial_number_is_in_dfu_mode(char *serial) {
    char *DFU = strstr(serial, "SRTG");
    return DFU != NULL;
}

bool dfu_serial_number_is_pwned(char *serial) {
    char *pwnd = strstr(serial, "PWND:");
    return pwnd != NULL;
}

bool dfu_serial_number_is_in_yolo_dfu(char *serial) {
    char *yolo = strstr(serial, "YOLO");
    return yolo != NULL;
}
