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

#include "MainPanel.hpp"

#include <wx/wx.h>
#include <wx/hyperlink.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"

#include "../events/event.hpp"
#include "../globals.h"
#include "../paleinfo.h"
#include "../gen/embedded/logo.h"

const char* deviceTextString = "No device connected";
const char* deviceTextString2 = "Please connect a device to get started.\nEnsure version range is 15.0+";

MainPanel::MainPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    wxInitAllImageHandlers();
    auto* root = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    auto* right = new wxBoxSizer(wxVERTICAL);

    wxMemoryInputStream stream(embedded_logo_png, embedded_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);

    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);

    m_deviceTitle = new wxStaticText(this, wxID_ANY, deviceTextString);
    m_deviceSubtitle = new wxStaticText(this, wxID_ANY, deviceTextString2);

    wxFont titleFont = m_deviceTitle->GetFont();
    titleFont.SetPointSize(titleFont.GetPointSize() + 6);
    titleFont.SetWeight(wxFONTWEIGHT_SEMIBOLD);
    m_deviceTitle->SetFont(titleFont);

    m_deviceSubtitle->SetForegroundColour(wxColour(160, 160, 160));

    auto* credtext = new wxStaticText(
        this,
        wxID_ANY,
        "Made by: asdfugil, claration, kok3shidoll, mineek, plooshi, staturnz\n\n"
        "Thanks to: itsnebulalol, llsc12, lrdsnow, dedbeddedbed, kirb, ehilwyma, "
        "opa334, 0x7ff, alfiecg24, sneko, sbingner, nikias, tihmstar, "
        "Checkra1n (Siguza, axi0mx, littlelailo et al.), Procursus (Hayden Seay, "
        "Cameron Katri, Keto et al.)",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    );

    auto* twitterLabel = new wxStaticText(this, wxID_ANY, "Twitter:");
    auto* twitter = new wxStaticText(this, wxID_ANY, "@palera1n");

    auto* websiteLabel = new wxStaticText(this, wxID_ANY, "Website:");
    auto* website = new wxStaticText(this, wxID_ANY, "https://palera.in");

    twitter->SetForegroundColour(wxColour(0, 120, 215));
    website->SetForegroundColour(wxColour(0, 120, 215));

    twitter->SetCursor(wxCursor(wxCURSOR_HAND));
    website->SetCursor(wxCursor(wxCURSOR_HAND));

    twitter->Bind(wxEVT_LEFT_DOWN, [](wxMouseEvent&)
    {
        wxLaunchDefaultBrowser("https://twitter.com/intent/follow?screen_name=palera1n");
    });

    website->Bind(wxEVT_LEFT_DOWN, [](wxMouseEvent&)
    {
        wxLaunchDefaultBrowser("https://palera.in");
    });

    auto* twitterRow = new wxBoxSizer(wxHORIZONTAL);
    twitterRow->Add(twitterLabel, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
    twitterRow->Add(twitter, 0, wxALIGN_CENTER_VERTICAL);

    auto* websiteRow = new wxBoxSizer(wxHORIZONTAL);
    websiteRow->Add(websiteLabel, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 5);
    websiteRow->Add(website, 0, wxALIGN_CENTER_VERTICAL);

    auto* logoText = new wxStaticText(this, wxID_ANY, "palera1n");

    auto* quickMode = new wxCheckBox(this, wxID_ANY, "Enable Quick Mode");
    auto* donateButton = new wxButton(this, wxID_ANY, "Donate");
    auto* optionsButton = new wxButton(this, wxID_ANY, "Options");

    m_startButton = new wxButton(this, wxID_ANY, "Start");
    m_startButton->Disable();

    quickMode->SetValue(palerain_flags & palerain_option_quick);

    quickMode->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_quick;
        else
            palerain_flags &= ~palerain_option_quick;
    });

    optionsButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowSettings();
    });

    m_startButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        if (frame->GetDeviceState().mode == DeviceMode::Normal)
            frame->ShowRecovery();
        else if (frame->GetDeviceState().mode == DeviceMode::Recovery)
            frame->ShowDfu();
    });

    left->Add(m_deviceTitle, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    left->Add(m_deviceSubtitle, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    left->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    left->Add(credtext, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);

    auto* linksCol = new wxBoxSizer(wxVERTICAL);
    linksCol->Add(twitterRow, 0);
    linksCol->Add(websiteRow, 0);

    auto* warningNote = new wxStaticText(this, wxID_ANY,
        "NOTE: Please ensure you've made a backup of your device before proceeding.\n",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    );

    wxFont noteFont = warningNote->GetFont();
    noteFont.SetStyle(wxFONTSTYLE_ITALIC);
    warningNote->SetFont(noteFont);
    warningNote->SetForegroundColour(wxColour(200, 80, 80));

    auto* linksRow = new wxBoxSizer(wxHORIZONTAL);
    linksRow->Add(linksCol, 0, wxLEFT | wxRIGHT | wxALIGN_TOP, 10);
    linksRow->AddStretchSpacer();
    linksRow->Add(warningNote, 0, wxRIGHT | wxALIGN_TOP, 10);

    left->Add(linksRow, 0, wxEXPAND | wxBOTTOM, 10);

    left->AddStretchSpacer();

    left->Add(new wxStaticLine(this), 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    #if __APPLE__
    auto* madeBy = new wxStaticText(this, wxID_ANY, "Made with 💖 by C (claration)");
    #else
    auto* madeBy = new wxStaticText(this, wxID_ANY, "Made with <3 by C (claration)");
    #endif

    auto* quickModeRow = new wxBoxSizer(wxHORIZONTAL);
    quickModeRow->Add(madeBy, 0, wxALIGN_CENTER_VERTICAL);
    quickModeRow->AddStretchSpacer();
    quickModeRow->Add(quickMode, 0, wxALIGN_CENTER_VERTICAL);

    left->Add(quickModeRow, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    right->Add(logo, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxLEFT | wxRIGHT, 10);
    right->Add(logoText, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);
    right->AddStretchSpacer();
    right->Add(donateButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    right->Add(optionsButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    right->Add(m_startButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    root->Add(left, 1, wxEXPAND);
    root->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
    root->Add(right, 0, wxEXPAND);

    SetSizer(root);
}

void MainPanel::SetDeviceState(const DeviceState& state)
{
    DevicePanel::SetDeviceState(state);

    if (m_deviceTitle && m_deviceSubtitle)
    {
        if (state.multipleDevices || state.connectedDeviceCount > 1)
        {
            m_deviceTitle->SetLabel("Multiple devices detected");
            m_deviceSubtitle->SetLabel(
                wxString::Format(
                    "%u devices connected. Please only connect one device.\n",
                    state.connectedDeviceCount
                )
            );
            m_startButton->Enable(false);
        }
        else if (!state.connected)
        {
            m_deviceTitle->SetLabel(deviceTextString);
            m_deviceSubtitle->SetLabel(deviceTextString2);
            m_startButton->Enable(false);
        }
        else
        {
            const std::string versionString =
                state.productVersion.empty() ? "Unknown" : state.productVersion;
            const std::string productString =
                state.displayName.empty() ? (state.productType.empty() ? "Unknown" : state.productType) : state.displayName;
            const std::string ecidString = state.ecid != 0 ? ("ECID: " + std::to_string(state.ecid)) : "ECID: Unknown";

            switch (state.mode)
            {
                case DeviceMode::Normal:
                    if (state.isSupported) {
                        m_deviceTitle->SetLabel(productString);
                        m_deviceSubtitle->SetLabel("Connected in normal mode - iOS " + versionString + "\n" + ecidString);
                        m_startButton->Enable();
                    } else {
                        m_deviceTitle->SetLabel(productString);
                        m_deviceSubtitle->SetLabel("Not supported - iOS " + versionString + "\n" + ecidString);
                        m_startButton->Enable(false);
                    }
                    break;
                case DeviceMode::Recovery:
                    if (state.isSupported) {
                        m_deviceTitle->SetLabel(productString);
                        m_deviceSubtitle->SetLabel("Connected in recovery mode\n" + ecidString);
                        m_startButton->Enable();
                    } else {
                        m_deviceTitle->SetLabel(productString);
                        m_deviceSubtitle->SetLabel("Not supported\n" + ecidString);
                        m_startButton->Enable(false);
                    }
                    break;
                case DeviceMode::DFU:
                    m_deviceTitle->SetLabel("DFU Mode device");
                    m_deviceSubtitle->SetLabel("Sorry, jailbreaking in DFU mode is not supported.\n");
                    m_startButton->Enable(false);
                    break;
                case DeviceMode::None:
                    m_deviceTitle->SetLabel(deviceTextString);
                    m_deviceSubtitle->SetLabel(deviceTextString2);
                    m_startButton->Enable(false);
                    break;
            }
        }
    }
}

#endif
