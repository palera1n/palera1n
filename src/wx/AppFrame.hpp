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

#ifndef APPFRAME_H
#define APPFRAME_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "../events/event.hpp"

#include "MainPanel.hpp"
#include "SettingsPanel.hpp"
#include "RecoveryPanel.hpp"
#include "DfuPanel.hpp"
#include "ExploitPanel.hpp"

void send_device_state(MainFrame* frame, const DeviceState& state);

class MainFrame : public wxFrame
{
public:
    MainFrame();

    void ShowMain();
    void ShowSettings();
    void ShowRecovery();
    void ShowDfu();
    void ShowExploit();

    const DeviceState& GetDeviceState() const { return m_state; }
    void UpdateDeviceState(const DeviceState& state);

    MainPanel* GetMainPanel() const { return m_main; }
    SettingsPanel* GetSettingsPanel() const { return m_settings; }
    RecoveryPanel* GetRecoveryPanel() const { return m_recovery; }
    DfuPanel* GetDfuPanel() const { return m_dfu; }
    ExploitPanel* GetExploitPanel() const { return m_exploit; }
private:
    MainPanel* m_main = nullptr;
    SettingsPanel* m_settings = nullptr;
    RecoveryPanel* m_recovery = nullptr;
    DfuPanel* m_dfu = nullptr;
    ExploitPanel* m_exploit = nullptr;

    DeviceState m_state;
};

class PalerainApp : public wxApp {
public:
    bool OnInit() override;
};

#endif // WITH_GUI

#endif // APPFRAME_H
