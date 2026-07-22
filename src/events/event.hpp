/*
 * palera1n - https://palera.in
 *
 * Copyright (C) 2026 palera1n team
 *
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#if defined(WITH_GUI) || defined(WITH_TUI)

#ifndef EVENT_H
#define EVENT_H

#include <cstdint>
#include <functional>
#include <string>

#ifdef WITH_GUI
# include <wx/wx.h>
#endif

enum class DeviceMode
{
    None,
    Normal,
    Recovery,
    DFU
};

struct DeviceState
{
    bool connected = false;
    bool isSupported = false;
    bool multipleDevices = false;
    DeviceMode mode = DeviceMode::None;
    std::string productVersion;
    std::string productType;
    std::string displayName;
    uint64_t ecid = 0;
    std::string udid;
    uint32_t connectedDeviceCount = 0;
};

using DeviceStateCallback = std::function<void(const DeviceState&)>;

void register_device_state_callback(DeviceStateCallback cb);
void ensure_device_event_system_started();
bool enter_recovery();
void exit_recovery();

#endif // EVENT_H

#endif // WITH_GUI || WITH_TUI
