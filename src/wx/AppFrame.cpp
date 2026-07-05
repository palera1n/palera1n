#ifdef WITH_GUI

#include "AppFrame.hpp"

#include <wx/wx.h>
#include "../utils.h"
#include "../event.hpp"

#include <thread>

#include <libimobiledevice/libimobiledevice.h>
#include <libimobiledevice/lockdown.h>
#include <libirecovery.h>

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
              "palera1n - Version beta " + wxString(PALERAIN_VERSION),
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

    std::thread([]()
    {
        idevice_event_subscribe(normal_device_event_cb, nullptr);

        while (true)
            sleep_ms(1000);
    }).detach();

    std::thread([]()
    {
        static irecv_device_event_context_t ctx = nullptr;

        irecv_device_event_subscribe(
            &ctx,
            recovery_device_event_cb,
            nullptr
        );

        while (true)
            sleep_ms(1000);
    }).detach();
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

    if (m_main)
        m_main->SetDeviceState(m_state);

    if (m_recovery)
        m_recovery->SetDeviceState(m_state);

    if (m_dfu)
        m_dfu->SetDeviceState(m_state);

    if (m_exploit)
        m_exploit->SetDeviceState(m_state);
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
