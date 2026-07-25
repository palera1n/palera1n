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

#ifdef WITH_GUI

#include "AppFrame.hpp"

#include <wx/wx.h>
#include "../globals.h"
#include "../events/event.hpp"

wxDEFINE_EVENT(EVT_DEVICE_STATE_UPDATE, wxCommandEvent);

void send_device_state(MainFrame* frame, const DeviceState& state)
{
    if (!wxTheApp) return;

    wxTheApp->CallAfter([frame, state]() {
        if (!frame) return;
        auto* evt = new wxCommandEvent(EVT_DEVICE_STATE_UPDATE);
        evt->SetClientData(new DeviceState(state));
        wxPostEvent(frame, *evt);
    });
}

MainFrame::MainFrame()
    : wxFrame(nullptr,
              wxID_ANY,
              "palera1n - Version " + wxString(PALERAIN_VERSION),
              wxDefaultPosition,
              wxSize(480, 348),
              wxDEFAULT_FRAME_STYLE & ~(wxMAXIMIZE_BOX | wxRESIZE_BORDER))
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    m_main = new MainPanel(this, this);
    m_settings = new SettingsPanel(this, this);
    m_recovery = new RecoveryPanel(this, this);
    m_dfu = new DfuPanel(this, this);
    m_exploit = new ExploitPanel(this, this);

    root->Add(m_main, 1, wxEXPAND);
    root->Add(m_settings, 1, wxEXPAND);
    root->Add(m_recovery, 1, wxEXPAND);
    root->Add(m_dfu, 1, wxEXPAND);
    root->Add(m_exploit, 1, wxEXPAND);

    SetSizer(root);

    Bind(EVT_DEVICE_STATE_UPDATE, [this](wxCommandEvent& e)
    {
        auto* state = static_cast<DeviceState*>(e.GetClientData());

        if (!state)
            return;

        UpdateDeviceState(*state);

        delete state;
    });

    ShowMain();

    register_device_state_callback([this](const DeviceState& state) {
        send_device_state(this, state);
    });
    ensure_device_event_system_started();
}

void MainFrame::ShowMain()
{
    m_main->Show();
    m_settings->Hide();
    m_recovery->Hide();
    m_dfu->Hide();
    m_exploit->Hide();
    Layout();
}

void MainFrame::ShowSettings()
{
    m_main->Hide();
    m_settings->Show();
    m_recovery->Hide();
    m_dfu->Hide();
    m_exploit->Hide();
    Layout();
}

void MainFrame::ShowRecovery()
{
    m_main->Hide();
    m_settings->Hide();
    m_recovery->Show();
    m_dfu->Hide();
    m_exploit->Hide();
    Layout();
}

void MainFrame::ShowDfu()
{
    m_main->Hide();
    m_settings->Hide();
    m_recovery->Hide();
    m_dfu->Show();
    m_dfu->LoadDevice(m_state.productType);
    m_exploit->Hide();
    Layout();
}

void MainFrame::ShowExploit()
{
    m_main->Hide();
    m_settings->Hide();
    m_recovery->Hide();
    m_dfu->Hide();
    m_exploit->Show();
    m_exploit->StartExploit();
    Layout();
}

void MainFrame::UpdateDeviceState(const DeviceState& state)
{
    m_state = state;

    if (m_main)     m_main->SetDeviceState(m_state);
    if (m_recovery) m_recovery->SetDeviceState(m_state);
    if (m_dfu)      m_dfu->SetDeviceState(m_state);
    if (m_exploit)  m_exploit->SetDeviceState(m_state);
}

bool PalerainApp::OnInit()
{
    #ifdef _WIN32
    wxTheApp->SetAppearance(wxApp::Appearance::Dark);
    #endif

    auto* frame = new MainFrame();
    frame->Show(true);
    return true;
}

wxIMPLEMENT_APP_NO_MAIN(PalerainApp);

#endif
