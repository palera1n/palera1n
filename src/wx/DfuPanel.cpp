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

#include "DfuPanel.hpp"

#include "AppFrame.hpp"
#include "DevicePanel.hpp"

#include "../events/event.hpp"
#include "../sequence.hpp"
#include "../globals.h"
#include "../paleinfo.h"

#include <thread>

#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include "../gen/embedded/atv_std_brd.h"
#include "../gen/embedded/ipad.h"
#include "../gen/embedded/ipadmini.h"
#include "../gen/embedded/iphone6s.h"
#include "../gen/embedded/iphone7.h"
#include "../gen/embedded/iphone8.h"
#include "../gen/embedded/iphonese.h"
#include "../gen/embedded/iphonex.h"
#include "../gen/embedded/ipodtouch.h"
#include "../gen/embedded/siriremote.h"
#include "../gen/embedded/logo.h"

DfuPanel::DfuPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    Bind(wxEVT_SHOW, &DfuPanel::OnShow, this);

    auto* root = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    wxMemoryInputStream stream(embedded_logo_png, embedded_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);
    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);
    auto* logoText = new wxStaticText(this, wxID_ANY, "DFU");
    auto* right = new wxBoxSizer(wxVERTICAL);

    m_headerText = new wxStaticText(this, wxID_ANY,
        "Time to put the device into DFU mode.",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    );
    left->Add(m_headerText, 0, wxTOP | wxLEFT | wxRIGHT, 10);

    m_devicePanel = new wxPanel(this);
    m_devicePanel->SetMinSize(wxSize(310, 260));

    m_deviceImage = new wxStaticBitmap(
        m_devicePanel,
        wxID_ANY,
        wxBitmap()
    );

    auto* deviceSizer = new wxBoxSizer(wxVERTICAL);
    deviceSizer->AddStretchSpacer(1);
    deviceSizer->Add(m_deviceImage, 0, wxALIGN_CENTER);
    deviceSizer->AddStretchSpacer(1);
    m_devicePanel->SetSizer(deviceSizer);

    left->Add(m_devicePanel, 1, wxEXPAND | wxTOP, 0);

    left->AddStretchSpacer(1);

    m_stepsSizer = new wxBoxSizer(wxVERTICAL);
    left->Add(m_stepsSizer, 0, wxEXPAND | wxBOTTOM | wxLEFT | wxRIGHT, 10);

    m_backButton = new wxButton(this, wxID_ANY, "Back");
    m_startButton = new wxButton(this, wxID_ANY, "Start");

    m_backButton->Bind(wxEVT_BUTTON, [frame, this](wxCommandEvent&)
    {
        Reboot();
        frame->ShowMain();
    });

    m_startButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
    {
        m_isEnteringDfu = true;
        m_backButton->Disable();
        m_startButton->Disable();

        StartSequence(m_sequence);
    });

    m_timer.Bind(wxEVT_TIMER, [this](wxTimerEvent&)
    {
        RunStep();
    });

    m_stagnentTimer.Bind(wxEVT_TIMER, [this](wxTimerEvent&)
    {
        m_headerText->SetLabel("Time to put the device into DFU mode.");
        m_backButton->Enable();
        m_startButton->Enable();
        GetMainFrame()->ShowExploit();
    });

    right->Add(logo, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxLEFT | wxRIGHT, 10);
    right->Add(logoText, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);
    right->AddStretchSpacer();
    right->Add(m_backButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    right->Add(m_startButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    root->Add(left, 1, wxEXPAND);
    root->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
    root->Add(right, 0, wxEXPAND);

    SetSizer(root);
}

void DfuPanel::SetDeviceState(const DeviceState& state)
{
    DevicePanel::SetDeviceState(state);

    if (!(this->IsShown()))
        return;

    if (m_isEnteringDfu && state.mode == DeviceMode::DFU)
    {
        m_isEnteringDfu = false;
        m_headerText->SetLabel("Device entered DFU mode successfully.");

        m_index = m_sequence.steps.size();
        m_stepRemaining = -1;
        m_actionExecutedIndex = -1;
        m_waitingForDfuTransition = true;

        const wxColour inactive =
            wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

        for (auto* lbl : m_stepLabels)
            lbl->SetForegroundColour(inactive);

        for (auto* lbl : m_buttonLabels)
            lbl->SetForegroundColour(inactive);

        if (!m_stagnentTimer.IsRunning())
            m_stagnentTimer.StartOnce(3000);
    }

    if (m_isEnteringDfu && state.mode == DeviceMode::Normal)
    {
        m_isEnteringDfu = false;
        m_backButton->Enable();
        m_startButton->Enable();
        GetMainFrame()->ShowRecovery();
        GetMainFrame()->GetRecoveryPanel()->SetStatusText("Hmm... It seems like the device didn't enter DFU mode.");
    }

    if (m_isEnteringDfu && state.mode == DeviceMode::Recovery)
    {
        m_isEnteringDfu = false;
        m_backButton->Enable();
        m_startButton->Enable();
        GetMainFrame()->ShowMain();
    }

    if (!state.connected)
    {
        if (m_isEnteringDfu || m_waitingForDfuTransition)
            return;

        m_isEnteringDfu = false;
        m_backButton->Enable();
        m_startButton->Enable();
        GetMainFrame()->ShowMain();
    }
}

void DfuPanel::OnShow(wxShowEvent& event)
{
    DevicePanel::OnShow(event);

    if (event.IsShown())
    {
        if (palerain_flags & palerain_option_quick) {
            m_isEnteringDfu = true;
            m_backButton->Disable();
            m_startButton->Disable();

            StartSequence(m_sequence);
        }
    }
}

void DfuPanel::LoadDevice(const std::string& productType)
{
    const wxColour active =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    const wxColour inactive =
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

    m_sequence = ParseSequence(productType);
    m_index = 0;

    m_deviceImage->SetBitmap(wxBitmap());

    for (auto* lbl : m_stepLabels)
        lbl->Destroy();

    m_stepLabels.clear();
    m_stepsSizer->Clear(false);

    for (auto* lbl : m_buttonLabels)
        lbl->Destroy();

    m_buttonLabels.clear();

    const unsigned char* imageData = nullptr;
    size_t imageSize = 0;

    if (m_sequence.imageName == "ipadmini")
    {
        imageData = embedded_ipadmini_png;
        imageSize = embedded_ipadmini_png_len;
    }
    else if (m_sequence.imageName == "ipad")
    {
        imageData = embedded_ipad_png;
        imageSize = embedded_ipad_png_len;
    }
    else if (m_sequence.imageName == "iphonese")
    {
        imageData = embedded_iphonese_png;
        imageSize = embedded_iphonese_png_len;
    }
    else if (m_sequence.imageName == "iphone6s")
    {
        imageData = embedded_iphone6s_png;
        imageSize = embedded_iphone6s_png_len;
    }
    else if (m_sequence.imageName == "iphone7")
    {
        imageData = embedded_iphone7_png;
        imageSize = embedded_iphone7_png_len;
    }
    else if (m_sequence.imageName == "iphone8")
    {
        imageData = embedded_iphone8_png;
        imageSize = embedded_iphone8_png_len;
    }
    else if (m_sequence.imageName == "iphonex")
    {
        imageData = embedded_iphonex_png;
        imageSize = embedded_iphonex_png_len;
    }
    else if (m_sequence.imageName == "ipodtouch")
    {
        imageData = embedded_ipodtouch_png;
        imageSize = embedded_ipodtouch_png_len;
    }
    else if (m_sequence.imageName == "siriremote")
    {
        imageData = embedded_siriremote_png;
        imageSize = embedded_siriremote_png_len;
    }
    else if (m_sequence.imageName == "atv_std_brd")
    {
        imageData = embedded_atv_std_brd_png;
        imageSize = embedded_atv_std_brd_png_len;
    }

    if (imageData)
    {
        wxMemoryInputStream stream(imageData, imageSize);

        wxImage img(stream, wxBITMAP_TYPE_PNG);

        if (img.IsOk())
        {
            img = img.Scale(
                m_sequence.imageWidth,
                m_sequence.imageHeight,
                wxIMAGE_QUALITY_HIGH
            );

            m_deviceImage->SetBitmap(wxBitmap(img));
        }
    }

    for (const auto& button : m_sequence.buttons)
    {
        auto* lbl = new wxStaticText(
            m_devicePanel,
            wxID_ANY,
            button.name
        );

        lbl->SetPosition(wxPoint(button.x, button.y));

        m_buttonLabels.push_back(lbl);
    }

    for (size_t i = 0; i < m_sequence.steps.size(); i++)
    {
        const auto& step = m_sequence.steps[i];

        wxString label = wxString::Format(
            "%zu. %s (%d)",
            i + 1,
            step.description,
            step.duration
        );

        auto* lbl = new wxStaticText(
            this,
            wxID_ANY,
            label
        );

        lbl->SetForegroundColour(i == 0 ? active : inactive);

        m_stepsSizer->Add(lbl, 0, wxTOP, 5);
        m_stepLabels.push_back(lbl);
    }

    m_devicePanel->Layout();
    m_stepsSizer->Layout();

    Layout();
    Fit();
    Refresh();
}

void DfuPanel::StartSequence(const DfuSequence& seq)
{
    m_sequence = seq;
    m_index = 0;
    m_stepRemaining = -1;
    m_actionExecutedIndex = -1;
    m_waitingForDfuTransition = false;

    RunStep();
}

void DfuPanel::RunStep()
{
    if (m_sequence.steps.empty())
        return;

    const wxColour active =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    const wxColour inactive =
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

    if (m_index < m_sequence.steps.size())
    {
        if (m_stepRemaining < 0)
        {
            m_stepRemaining = m_sequence.steps[m_index].duration;
        }
        else if (m_stepRemaining > 0)
        {
            --m_stepRemaining;
        }

        if (m_stepRemaining == 0)
        {
            ++m_index;

            if (m_index < m_sequence.steps.size())
                m_stepRemaining = m_sequence.steps[m_index].duration;
        }
    }

    if (m_index >= m_sequence.steps.size())
    {
        for (auto* lbl : m_stepLabels)
            lbl->SetForegroundColour(inactive);

        m_timer.Stop();
        Refresh();
        return;
    }

    for (size_t i = 0; i < m_stepLabels.size(); ++i)
    {
        auto* lbl = m_stepLabels[i];
        const auto& step = m_sequence.steps[i];

        lbl->SetForegroundColour(i == m_index ? active : inactive);

        int remaining = step.duration;

        if (i == m_index)
            remaining = m_stepRemaining;
        else if (i < m_index)
            remaining = 0;

        lbl->SetLabel(wxString::Format(
            "%zu. %s (%d)",
            i + 1,
            step.description,
            remaining
        ));
    }

    for (auto* lbl : m_buttonLabels)
        lbl->SetForegroundColour(inactive);

    for (const auto& buttonId : m_sequence.steps[m_index].activeButtons)
    {
        for (const auto& btn : m_sequence.buttons)
        {
            if (btn.id != buttonId)
                continue;

            for (auto* lbl : m_buttonLabels)
            {
                if (lbl->GetLabel() == btn.name)
                    lbl->SetForegroundColour(active);
            }
        }
    }

    const auto& step = m_sequence.steps[m_index];

    if (!step.action.empty() &&
        m_actionExecutedIndex != static_cast<int>(m_index))
    {
        m_actionExecutedIndex = m_index;

        if (step.action == "reboot")
            Reboot();
    }

    Layout();
    Refresh();
    Update();

    m_timer.StartOnce(1000);
}

void DfuPanel::Reboot()
{
    std::thread([this]()
    {
        const auto state = GetDeviceState();

        if (!state.connected || state.mode != DeviceMode::Recovery)
            return;

        exit_recovery();
    }).detach();
}

#endif // WITH_GUI
