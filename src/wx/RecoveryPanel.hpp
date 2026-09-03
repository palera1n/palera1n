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

#ifndef RECOVERYPANEL_H
#define RECOVERYPANEL_H

#ifdef WITH_GUI

#include <wx/wx.h>
#include "DevicePanel.hpp"

class RecoveryPanel;
class RecoveryPanel : public DevicePanel
{
public:
    explicit RecoveryPanel(MainFrame* frame, wxWindow* parent);
    void SetDeviceState(const DeviceState& state) override;
    void SetStatusText(const wxString& text);
    void EnterRecoveryMode();
    void OnShow(wxShowEvent& event) override;
private:
    wxButton* m_backButton = nullptr;
    wxButton* m_nextButton = nullptr;
    wxStaticText* m_statusText = nullptr;
    wxTimer m_timer;
    bool m_isEnteringRecovery = false;
};


#endif // WITH_GUI

#endif // RECOVERYPANEL_H
