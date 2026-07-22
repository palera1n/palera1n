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

#ifdef WITH_TUI

#ifndef TUI_HPP
#define TUI_HPP

#include <memory>
#include <mutex>
#include <vector>

#include "../events/event.hpp"
#include "TuiPanel.hpp"

class TuiPanel;
class TuiMainPanel;
class TuiSettingsPanel;
class TuiRecoveryPanel;
class TuiDfuPanel;
class TuiExploitPanel;

class TuiFrame {
public:
    TuiFrame();
    ~TuiFrame();

    void Run();

    void ShowMain(int selected = 1);
    void ShowSettings(int selected = 5);
    void ShowRecovery(int selected = 1);
    void ShowDfu(int selected = 1);
    void ShowExploit(int selected = 0);

    PanelState GetCurrentPanel() const;
    int GetSelected() const;
    void SetSelected(int selected);

    bool IsRunning() const;
    void Stop();

    DeviceState GetDeviceState() const;
    void UpdateDeviceState(const DeviceState& state);

    TuiMainPanel* GetMainPanel() const;
    TuiSettingsPanel* GetSettingsPanel() const;
    TuiRecoveryPanel* GetRecoveryPanel() const;
    TuiDfuPanel* GetDfuPanel() const;
    TuiExploitPanel* GetExploitPanel() const;

private:
    void InitPanels();
    void InitDeviceEventListeners();
    void ActivatePanelIfNeeded(PanelState previous_panel);
    void DrawUi();
    void HandleInput(int ch, int start_y, int start_x);

    PanelState m_currentPanel = PANEL_MAIN;
    int m_selected = 1;
    bool m_running = true;

    DeviceState m_device;
    mutable std::mutex m_deviceMutex;

    std::vector<std::unique_ptr<TuiPanel>> m_panels;
};

void ui_run(void);
void update_tui_device_state(const DeviceState* new_state);

#endif // TUI_HPP

#endif // WITH_TUI
