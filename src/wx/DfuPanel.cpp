#ifdef WITH_GUI

#include "DfuPanel.hpp"

#include "AppFrame.hpp"
#include "DevicePanel.hpp"

#include "../state.hpp"
#include "../sequence.hpp"
#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"

#include <thread>

#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>

#include "../gen/images/atv_std_brd.h"
#include "../gen/images/ipad.h"
#include "../gen/images/ipadmini.h"
#include "../gen/images/iphone6s.h"
#include "../gen/images/iphone7.h"
#include "../gen/images/iphone8.h"
#include "../gen/images/iphonese.h"
#include "../gen/images/iphonex.h"
#include "../gen/images/ipodtouch.h"
#include "../gen/images/siriremote.h"

#include <libirecovery.h>

DfuPanel::DfuPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    Bind(wxEVT_SHOW, &DfuPanel::OnShow, this);
    auto* root = new wxBoxSizer(wxVERTICAL);

    m_headerText = new wxStaticText(this, wxID_ANY, "Time to put the device into DFU mode. Locate the buttons as marked below\non your device and check the instructions on the right.");
    root->Add(m_headerText, 0, wxALL, 10);

    auto* contentRow = new wxBoxSizer(wxHORIZONTAL);

    m_devicePanel = new wxPanel(this);
    m_devicePanel->SetMinSize(wxSize(310, 260));

    m_deviceImage = new wxStaticBitmap(
        m_devicePanel,
        wxID_ANY,
        wxBitmap()
    );

    contentRow->Add(m_devicePanel, 1, wxTOP, 10);
    m_stepsSizer = new wxBoxSizer(wxVERTICAL);
    contentRow->Add(m_stepsSizer, 2, wxEXPAND | wxALL, 0);

    root->Add(contentRow, 1, wxEXPAND);

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
        m_headerText->SetLabel("Time to put the device into DFU mode. Locate the buttons as marked below\non your device and check the instructions on the right.");
        m_backButton->Enable();
        m_startButton->Enable();
        GetMainFrame()->ShowExploit();
    });

    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(m_backButton, 0, wxRIGHT | wxLEFT | wxBOTTOM, 12);
    bottomRow->Add(m_startButton, 0, wxRIGHT | wxLEFT | wxBOTTOM, 12);

    root->Add(bottomRow, 0, wxEXPAND);

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
        m_headerText->SetLabel("Device entered DFU mode successfully.\n");

        m_index = m_sequence.steps.size();
        m_stepRemaining = -1;
        m_actionExecutedIndex = -1;

        const wxColour kInactive =
            wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

        for (auto* lbl : m_stepLabels)
            lbl->SetForegroundColour(kInactive);

        for (auto* lbl : m_buttonLabels)
            lbl->SetForegroundColour(kInactive);

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
}

void DfuPanel::OnShow(wxShowEvent& event)
{
    if (event.IsShown())
    {
        if (palerain_flags & palerain_option_quick) {
            m_isEnteringDfu = true;
            m_backButton->Disable();
            m_startButton->Disable();

            StartSequence(m_sequence);
        }
    }

    DevicePanel::OnShow(event);
}

void DfuPanel::LoadDevice(const std::string& productType)
{
    const wxColour kActive =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    const wxColour kInactive =
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
        imageData = images_ipadmini_png;
        imageSize = images_ipadmini_png_len;
    }
    else if (m_sequence.imageName == "ipad")
    {
        imageData = images_ipad_png;
        imageSize = images_ipad_png_len;
    }
    else if (m_sequence.imageName == "iphonese")
    {
        imageData = images_iphonese_png;
        imageSize = images_iphonese_png_len;
    }
    else if (m_sequence.imageName == "iphone6s")
    {
        imageData = images_iphone6s_png;
        imageSize = images_iphone6s_png_len;
    }
    else if (m_sequence.imageName == "iphone7")
    {
        imageData = images_iphone7_png;
        imageSize = images_iphone7_png_len;
    }
    else if (m_sequence.imageName == "iphone8")
    {
        imageData = images_iphone8_png;
        imageSize = images_iphone8_png_len;
    }
    else if (m_sequence.imageName == "iphonex")
    {
        imageData = images_iphonex_png;
        imageSize = images_iphonex_png_len;
    }
    else if (m_sequence.imageName == "ipodtouch")
    {
        imageData = images_ipodtouch_png;
        imageSize = images_ipodtouch_png_len;
    }
    else if (m_sequence.imageName == "siriremote")
    {
        imageData = images_siriremote_png;
        imageSize = images_siriremote_png_len;
    }
    else if (m_sequence.imageName == "atv_std_brd")
    {
        imageData = images_atv_std_brd_png;
        imageSize = images_atv_std_brd_png_len;
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
            m_deviceImage->SetPosition(wxPoint(m_sequence.imageOffsetX, 0));
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

        lbl->SetForegroundColour(i == 0 ? kActive : kInactive);

        m_stepsSizer->Add(lbl, 0, wxBOTTOM, 5);
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

    RunStep();
    m_timer.StartOnce(1000);
}

void DfuPanel::RunStep()
{
    if (m_sequence.steps.empty())
        return;

    const wxColour kActive =
        wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);

    const wxColour kInactive =
        wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT);

    if (m_index >= m_sequence.steps.size())
    {
        for (auto* lbl : m_stepLabels)
            lbl->SetForegroundColour(kInactive);

        Layout();
        Refresh();

        m_timer.Stop();
        return;
    }

    const auto& step = m_sequence.steps[m_index];

    if (m_stepRemaining < 0)
        m_stepRemaining = step.duration;

    if (m_stepRemaining > 0)
        --m_stepRemaining;

    bool finished = (m_stepRemaining == 0);
    size_t completedIndex = m_index;

    if (finished)
    {
        ++m_index;
        m_stepRemaining = -1;
    }

    for (size_t i = 0; i < m_stepLabels.size(); i++)
    {
        auto* lbl = m_stepLabels[i];

        if (i == m_index)
        {
            lbl->SetForegroundColour(kActive);

            lbl->SetLabel(wxString::Format(
                "%zu. %s (%d)",
                i + 1,
                m_sequence.steps[i].description,
                m_stepRemaining < 0 ? m_sequence.steps[i].duration : m_stepRemaining
            ));
        }
        else if (finished && i == completedIndex)
        {
            lbl->SetForegroundColour(kInactive);

            lbl->SetLabel(wxString::Format(
                "%zu. %s (0)",
                i + 1,
                m_sequence.steps[i].description
            ));
        }
        else
        {
            lbl->SetForegroundColour(kInactive);
        }
    }

    for (auto* lbl : m_buttonLabels)
        lbl->SetForegroundColour(kInactive);

    const auto& activeStep =
        (m_index < m_sequence.steps.size())
            ? m_sequence.steps[m_index]
            : m_sequence.steps.back();

    for (const auto& buttonId : activeStep.activeButtons)
    {
        for (const auto& btn : m_sequence.buttons)
        {
            if (btn.id == buttonId)
            {
                for (auto* lbl : m_buttonLabels)
                {
                    if (lbl->GetLabel() == btn.name)
                        lbl->SetForegroundColour(kActive);
                }
            }
        }
    }

    if (m_index < m_sequence.steps.size())
    {
        const auto& activeStep = m_sequence.steps[m_index];

        if (!activeStep.action.empty() && m_actionExecutedIndex != m_index)
        {
            m_actionExecutedIndex = m_index;

            if (activeStep.action == "reboot")
            {
                Reboot();
            }
        }
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

        irecv_client_t client = nullptr;

        int attempts = 0;
        const int max_attempts = 8;

        while (attempts < max_attempts)
        {
            if (irecv_open_with_ecid(&client, state.ecid) == IRECV_E_SUCCESS)
                break;

            attempts++;
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        if (!client)
        {
            return;
        }
        irecv_setenv(client, "auto-boot", "true");
        irecv_saveenv(client);
        irecv_reboot(client);

        irecv_close(client);
    }).detach();
}

#endif // WITH_GUI
