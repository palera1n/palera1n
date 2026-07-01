#ifdef WITH_GUI

#include "RecoveryPanel.hpp"

#include <wx/wx.h>

#include <thread>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"

#include "../state.hpp"
#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

RecoveryPanel::RecoveryPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    Bind(wxEVT_SHOW, &RecoveryPanel::OnShow, this);
    auto* root = new wxBoxSizer(wxVERTICAL);

    root->Add(new wxStaticText(this, wxID_ANY,
        "The device needs to be put into DFU mode to apply the jailbreak. This is a\n"
        "manual process and we will guide you through it.\n"
        "In order to prevent filesystem corruption through hard reset, the device will\n"
        "be put into recovery mode first. Click next when you are ready."
    ), 0, wxALL, 10);

    m_statusText = new wxStaticText(this, wxID_ANY, "");
    root->Add(m_statusText, 0, wxALL, 10);

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

    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(m_backButton, 0, wxRIGHT | wxBOTTOM, 12);
    bottomRow->Add(m_nextButton, 0, wxRIGHT | wxLEFT | wxBOTTOM, 12);

    root->AddStretchSpacer();
    root->Add(bottomRow, 0, wxEXPAND);

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
        if (!deviceState.connected)
            return;

        idevice_t device = nullptr;
        lockdownd_client_t client = nullptr;

        if (idevice_new(&device, deviceState.udid.c_str()) != IDEVICE_E_SUCCESS)
            return;

        if (lockdownd_client_new_with_handshake(
            device,
            &client,
            "palera1n") != LOCKDOWN_E_SUCCESS)
        {
            idevice_free(device);
            return;
        }

        m_isEnteringRecovery = true;

        lockdownd_enter_recovery(client);

        lockdownd_client_free(client);
        idevice_free(device);
    }).detach();
}

void RecoveryPanel::SetStatusText(const wxString& text)
{
    m_statusText->SetLabel(text);
}

#endif
