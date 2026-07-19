#ifdef WITH_GUI

#include "RecoveryPanel.hpp"

#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include <thread>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"

#include "../events/event.hpp"
#include "../globals.h"
#include "../paleinfo.h"
#include "../gen/embedded/logo.h"

RecoveryPanel::RecoveryPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    Bind(wxEVT_SHOW, &RecoveryPanel::OnShow, this);
    auto* root = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    wxMemoryInputStream stream(embedded_logo_png, embedded_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);
    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);
    auto* logoText = new wxStaticText(this, wxID_ANY, "Recovery");
    auto* right = new wxBoxSizer(wxVERTICAL);

    m_statusText = new wxStaticText(this, wxID_ANY, "");
    m_backButton = new wxButton(this, wxID_ANY, "Back");
    m_nextButton = new wxButton(this, wxID_ANY, "Next");

    m_backButton->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowMain();
    });
    m_nextButton->Bind(wxEVT_BUTTON, [this](wxCommandEvent&)
    {
        m_statusText->SetLabel("Entering recovery mode...");
        m_backButton->Disable();
        m_nextButton->Disable();
        EnterRecoveryMode();
    });
    m_timer.Bind(wxEVT_TIMER, [this](wxTimerEvent&)
    {
        m_backButton->Enable();
        m_nextButton->Enable();
        m_isEnteringRecovery = false;

        GetMainFrame()->ShowDfu();
        m_statusText->SetLabel("");
    });

    left->Add(
        new wxStaticText(this, wxID_ANY,
        "The device needs to be put into DFU mode to apply the jailbreak. This is a manual process and we will guide you through it.",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    ), 0, wxALL, 10);
    left->Add(m_statusText, 0, wxALL, 10);

    right->Add(logo, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxLEFT | wxRIGHT, 10);
    right->Add(logoText, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);
    right->AddStretchSpacer();
    right->Add(m_backButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
    right->Add(m_nextButton, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    root->Add(left, 1, wxEXPAND);
    root->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
    root->Add(right, 0, wxEXPAND);

    SetSizer(root);
}

void RecoveryPanel::SetDeviceState(const DeviceState& state)
{
    DevicePanel::SetDeviceState(state);

    if (!(this->IsShown()))
        return;

    if (m_isEnteringRecovery && state.mode == DeviceMode::Recovery)
    {
        m_statusText->SetLabel("Device is now in recovery mode.");
        m_timer.StartOnce(3000);
    }

    if (m_isEnteringRecovery && state.mode == DeviceMode::Normal)
    {
        m_isEnteringRecovery = false;
        m_statusText->SetLabel("Hmm... It seems like the device didn't enter recovery mode.");
        m_backButton->Enable();
        m_nextButton->Enable();
    }

    if (!m_isEnteringRecovery && state.mode == DeviceMode::None)
    {
        m_backButton->Enable();
        m_nextButton->Enable();
        GetMainFrame()->ShowMain();
    }
}

void RecoveryPanel::OnShow(wxShowEvent& event)
{
    if (event.IsShown())
    {
        if (palerain_flags & palerain_option_quick)
        {
            m_statusText->SetLabel("Entering recovery mode...");
            m_backButton->Disable();
            m_nextButton->Disable();
            EnterRecoveryMode();
        }
    }

    DevicePanel::OnShow(event);
}

void RecoveryPanel::EnterRecoveryMode()
{
    std::thread([this]()
    {
        const auto deviceState = GetDeviceState();
        if (!deviceState.connected) return;
        m_isEnteringRecovery = true;
        bool success = enter_recovery();
        if (!success)
        {
            wxTheApp->CallAfter([this]()
            {
                m_isEnteringRecovery = false;
                m_statusText->SetLabel("Failed to enter recovery mode.");
                m_backButton->Enable();
                m_nextButton->Enable();
            });
        }
    }).detach();
}

void RecoveryPanel::SetStatusText(const wxString& text)
{
    m_statusText->SetLabel(text);
}

#endif
