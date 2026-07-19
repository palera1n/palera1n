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

    if (!m_isEnteringDfu && !state.connected)
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

        lbl->SetForegroundColour(i == 0 ? kActive : kInactive);

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

        exit_recovery();
    }).detach();
}

#endif // WITH_GUI
