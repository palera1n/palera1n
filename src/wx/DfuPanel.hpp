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

#ifndef DFUPANEL_H
#define DFUPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"
#include "../events/event.hpp"
#include "../sequence.hpp"

class DfuPanel;
class DfuPanel : public DevicePanel
{
public:
    explicit DfuPanel(MainFrame* frame, wxWindow* parent);
    void SetDeviceState(const DeviceState& state) override;
    void OnShow(wxShowEvent& event) override;
    void LoadDevice(const std::string& productType);
    void StartSequence(const DfuSequence& seq);
    void RunStep();
    void Reboot();
private:
    wxStaticText* m_headerText = nullptr;
    wxButton* m_backButton = nullptr;
    wxButton* m_startButton = nullptr;

    wxBoxSizer* m_stepsSizer = nullptr;

    std::vector<wxStaticText*> m_stepLabels;

    DfuSequence m_sequence;
    size_t m_index = 0;
    int m_stepRemaining = 0;

    wxTimer m_timer;
    wxTimer m_stagnentTimer;
    bool m_isEnteringDfu = false;

    wxPanel* m_devicePanel = nullptr;
    wxStaticBitmap* m_deviceImage = nullptr;
    std::vector<wxStaticText*> m_buttonLabels;
    size_t m_actionExecutedIndex = std::numeric_limits<size_t>::max();
};


#endif // WITH_GUI

#endif // DFUPANEL_H
