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

#ifndef LIBOPENRA1N__UTILS_H
#define LIBOPENRA1N__UTILS_H

#include <stdint.h>
#include <stdbool.h>

#define RESET_COLOR   "\033[0m"
#define RED_COLOR     "\033[31m"
#define GREEN_COLOR   "\033[32m"
#define YELLOW_COLOR  "\033[33m"
#define BLUE_COLOR    "\033[34m"
#define MAGENTA_COLOR "\033[35m"
#define CYAN_COLOR    "\033[36m"
#define WHITE_COLOR   "\033[37m"
#define GRAY_COLOR    "\033[90m"

extern uint8_t gDebugLevel;
extern bool gSilentLogs;
extern bool gColoredLogs;

typedef enum {
    LOG_INFO,
    LOG_ERROR,
    LOG_VERBOSE
} log_level_t;

#ifdef __cplusplus
extern "C" {
#endif

void log_write_internal(log_level_t level, const char* func, const char* file, int line, const char *fmt, ...);

#ifdef __cplusplus
}
#endif

#define LOG(fmt, ...)           log_write_internal(LOG_INFO, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...)     log_write_internal(LOG_ERROR, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...)     log_write_internal(LOG_VERBOSE, __func__, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

void sleep_ms(unsigned ms);

#endif // LIBOPENRA1N__UTILS_H
