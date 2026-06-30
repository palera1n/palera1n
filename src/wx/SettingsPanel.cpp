#ifdef WITH_GUI

#include "SettingsPanel.hpp"

#include <wx/wx.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"
#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"

SettingsPanel::SettingsPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    auto* root = new wxBoxSizer(wxVERTICAL);

    root->Add(new wxStaticText(this, wxID_ANY,
        "You may set the following options. If you don't know what they mean you'll\n"
        "probably have no reason to set them."
    ), 0, wxALL, 10);

    auto* optionSafemode = new wxCheckBox(this, wxID_ANY, "Safe Mode");
    auto* optionVerbose = new wxCheckBox(this, wxID_ANY, "Verbose Boot");
    auto* optionRevert = new wxCheckBox(this, wxID_ANY, "Restore System");
    auto* optionDarkBlockchain = new wxCheckBox(this, wxID_ANY, "Dark Blockchain");

    optionSafemode->SetValue(palerain_flags & palerain_option_safemode);
    optionVerbose->SetValue(palerain_flags & palerain_option_verbose_boot);
    optionRevert->SetValue(palerain_flags & palerain_option_force_revert);
    optionDarkBlockchain->SetValue(palerain_flags & palerain_option_flower_chain);

    optionSafemode->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_safemode;
        else
            palerain_flags &= ~palerain_option_safemode;
    });

    optionVerbose->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_verbose_boot;
        else
            palerain_flags &= ~palerain_option_verbose_boot;
    });

    optionRevert->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_force_revert;
        else
            palerain_flags &= ~palerain_option_force_revert;
    });

    optionDarkBlockchain->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_flower_chain;
        else
            palerain_flags &= ~palerain_option_flower_chain;
    });

    root->Add(optionSafemode, 0, wxLEFT | wxRIGHT, 10);
    root->Add(optionVerbose, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    root->Add(new wxStaticText(this, wxID_ANY, "Boot Arguments:"), 0, wxALL, 10);

    auto* bootArgs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    bootArgs->SetValue(wxString(boot_args, wxConvUTF8));
    bootArgs->Bind(wxEVT_TEXT, [&](wxCommandEvent& e)
    {
        wxString value = e.GetString();
        if (value.length() > (sizeof(boot_args) - 0x20)) {
            LOG_ERROR("Boot arguments too long");
            return;
        }
        snprintf(boot_args, sizeof(boot_args), "%s", value.ToStdString().c_str());
    });

    root->Add(bootArgs, 0, wxLEFT | wxRIGHT | wxEXPAND, 30);
    root->Add(optionRevert, 0, wxLEFT | wxRIGHT | wxTOP, 10);
    root->Add(optionDarkBlockchain, 0, wxLEFT | wxRIGHT | wxTOP, 10);

    root->AddStretchSpacer();

    auto* back = new wxButton(this, wxID_ANY, "Back");

    back->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowMain();
    });

    auto* bottomRow = new wxBoxSizer(wxHORIZONTAL);
    bottomRow->AddStretchSpacer();
    bottomRow->Add(back, 0, wxRIGHT | wxLEFT | wxBOTTOM, 12);

    root->Add(bottomRow, 0, wxEXPAND);

    SetSizer(root);
}

#endif // WITH_GUI
