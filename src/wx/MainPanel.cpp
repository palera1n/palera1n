#ifdef WITH_GUI

#include "MainPanel.hpp"

#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"
#include "../state.hpp"

#include "../gen/images/logo.h"

const char* deviceTextString = "Connect your iPhone, iPod touch, iPad, or AppleTV to begin.";

MainPanel::MainPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    wxInitAllImageHandlers();

    wxMemoryInputStream stream(images_logo_png, images_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);

    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);

    auto* titletext = new wxStaticText(this, wxID_ANY, "Welcome to palera1n!");
    wxFont titlefont = titletext->GetFont();
    titlefont.SetWeight(wxFONTWEIGHT_BOLD);
    titletext->SetFont(titlefont);

    m_deviceText = new wxStaticText(
        this,
        wxID_ANY,
        deviceTextString
    );

    auto* left = new wxBoxSizer(wxVERTICAL);
    left->Add(titletext, 0, wxTOP | wxLEFT, 10);
    left->Add(m_deviceText, 0, wxTOP | wxLEFT, 10);

    auto* row = new wxBoxSizer(wxHORIZONTAL);
    row->Add(left, 0, wxALIGN_TOP);
    row->AddStretchSpacer();
    row->Add(logo, 0, wxTOP | wxRIGHT, 10);

    root->Add(row, 0, wxEXPAND);

    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    auto* credtext = new wxStaticText(this, wxID_ANY,
        "Made by: asdfugil, kok3shidoll, claration, mineek, staturnz\n\n"
        "Thanks to: itsnebulalol, llsc12, lrdsnow, nikias (libimobiledevice), Checkra1n\n"
        "(Siguza, axi0mx, littlelailo et al.), Procursus (Hayden Seay, Cameron Katri,\n"
        "Keto et al.)\n\n"
        "With <3 from C (claration)");

    root->Add(credtext, 0, wxLEFT | wxRIGHT, 10);

    root->Add(new wxStaticLine(this), 0, wxEXPAND | wxALL, 10);

    auto* notetext = new wxStaticText(this, wxID_ANY,
        "NOTE: Please ensure you have a backup of your device before applying the\n"
        "jailbreak. While data loss is unlikely, we won't be responsible if something goes\n"
        "wrong. Use at your own risk.");

    wxFont notefont = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    #ifdef __APPLE__
    notefont.SetPointSize(12);
    #else
    notefont.SetPointSize(9);
    #endif
    notefont.SetStyle(wxFONTSTYLE_ITALIC);
    notetext->SetFont(notefont);

    root->Add(notetext, 0, wxLEFT | wxRIGHT, 10);

    root->AddStretchSpacer();

    auto* btn = new wxButton(this, wxID_ANY, "Options");
    m_startButton = new wxButton(this, wxID_ANY, "Start");
    m_startButton->Disable();

    btn->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowSettings();
    });

    m_startButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        if (frame->GetDeviceState().mode == DeviceMode::Normal)
        {
            frame->ShowRecovery();
        }
        else if (frame->GetDeviceState().mode == DeviceMode::Recovery)
        {
            frame->ShowDfu();
        }
    });

    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(btn, 0, wxRIGHT | wxBOTTOM, 10);
    bottomRow->Add(m_startButton, 0, wxRIGHT | wxLEFT | wxBOTTOM, 10);

    root->Add(bottomRow, 0, wxEXPAND);

    SetSizer(root);
}

void MainPanel::SetDeviceState(const DeviceState& state)
{
    DevicePanel::SetDeviceState(state);

    if (m_deviceText)
    {
        if (!state.connected)
        {
            m_deviceText->SetLabel(deviceTextString);
            m_startButton->Enable(false);
        }
        else
        {
            const std::string versionString =
                state.productVersion.empty() ? "Unknown" : state.productVersion;
            const std::string productString =
                state.productType.empty() ? "Unknown" : state.productType;
            const std::string ecidString = state.ecid != 0 ? ("ECID: " + std::to_string(state.ecid)) : "ECID: Unknown";

            switch (state.mode)
            {
                case DeviceMode::Normal:
                    if (state.isSupported) {
                        m_deviceText->SetLabel("Connected " + productString + " (iOS " + versionString + ") in normal mode.\n" + ecidString);
                        m_startButton->Enable();
                    } else {
                        m_deviceText->SetLabel("Sorry, " + productString + " (iOS " + versionString + ") is not supported.\n" + ecidString);
                        m_startButton->Enable(false);
                    }
                    break;
                case DeviceMode::Recovery:
                    if (state.isSupported) {
                        m_deviceText->SetLabel("Connected " + productString + " in recovery mode.\n" + ecidString);
                        m_startButton->Enable();
                    } else {
                        m_deviceText->SetLabel("Sorry, " + productString + " is not supported.\n" + ecidString);
                        m_startButton->Enable(false);
                    }
                    break;
                case DeviceMode::DFU:
                    m_deviceText->SetLabel("Sorry, DFU mode is not supported.");
                    m_startButton->Enable(false);
                    break;
                case DeviceMode::None:
                    m_deviceText->SetLabel(deviceTextString);
                    m_startButton->Enable(false);
                    break;
            }
        }
    }
}

#endif
