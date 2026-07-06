#ifdef WITH_GUI

#include "SettingsPanel.hpp"

#include <wx/wx.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"
#include "../utils.h"
#include "../globals.h"
#include "../paleinfo.h"
#include "../gen/images/logo.h"

SettingsPanel::SettingsPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    auto* root = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    wxMemoryInputStream stream(images_logo_png, images_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);
    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);
    auto* logoText = new wxStaticText(this, wxID_ANY, "Options");
    auto* right = new wxBoxSizer(wxVERTICAL);

    left->Add(new wxStaticText(this, wxID_ANY,
        "You may set the following options. If you don't know what they mean you'll probably have no reason to set them.",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    ), 0, wxALL, 10);

    auto* optionSafemode = new wxCheckBox(this, wxID_ANY, "Safe Mode");
    auto* optionVerbose = new wxCheckBox(this, wxID_ANY, "Verbose Boot");
    auto* optionRevert = new wxCheckBox(this, wxID_ANY, "Restore System");
    auto* optionDarkBlockchain = new wxCheckBox(this, wxID_ANY, "Dark Blockchain");
    auto* bootArgs = new wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
    auto* rootType = new wxRadioBox(
        this,
        wxID_ANY,
        "Jailbreak Type",
        wxDefaultPosition,
        wxDefaultSize,
        2,
        (const wxString[]){
            "Rootless",
            "Rootful"
        },
        1,
        wxRA_SPECIFY_COLS
    );

    auto* setupMode = new wxRadioBox(
        this,
        wxID_ANY,
        "Rootful Options",
        wxDefaultPosition,
        wxDefaultSize,
        3,
        (const wxString[]){
            "Boot",
            "Create FakeFS",
            "Create BindFS"
        },
        1,
        wxRA_SPECIFY_COLS
    );

    optionSafemode->SetValue(palerain_flags & palerain_option_safemode);
    optionVerbose->SetValue(palerain_flags & palerain_option_verbose_boot);
    optionRevert->SetValue(palerain_flags & palerain_option_force_revert);
    optionDarkBlockchain->SetValue(palerain_flags & palerain_option_flower_chain);
    bootArgs->SetValue(wxString(boot_args, wxConvUTF8));
    if (palerain_flags & palerain_option_rootful)
        rootType->SetSelection(1);
    else
        rootType->SetSelection(0);

    if (palerain_flags & palerain_option_setup_rootful)
        setupMode->SetSelection(1);
    else if (palerain_flags & palerain_option_setup_partial_root)
        setupMode->SetSelection(2);
    else
        setupMode->SetSelection(0);
    setupMode->Enable(rootType->GetSelection() == 1);

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
    bootArgs->Bind(wxEVT_TEXT, [&](wxCommandEvent& e)
    {
        wxString value = e.GetString();
        if (value.length() > (sizeof(boot_args) - 0x20)) {
            LOG_ERROR("Boot arguments too long");
            return;
        }
        snprintf(boot_args, sizeof(boot_args), "%s", value.ToStdString().c_str());
    });
    rootType->Bind(wxEVT_RADIOBOX, [setupMode](wxCommandEvent& e)
    {
        palerain_flags &= ~(palerain_option_rootless | palerain_option_rootful);
        palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);

        setupMode->Enable(e.GetSelection() == 1);
        setupMode->SetSelection(0);

        if (e.GetSelection() == 0)
            palerain_flags |= palerain_option_rootless;
        else
            palerain_flags |= palerain_option_rootful;
    });

    setupMode->Bind(wxEVT_RADIOBOX, [&](wxCommandEvent& e)
    {
        palerain_flags &= ~(palerain_option_setup_rootful | palerain_option_setup_partial_root);

        if (e.GetSelection() == 1)
            palerain_flags |= palerain_option_setup_rootful;
        else if (e.GetSelection() == 2)
            palerain_flags |= palerain_option_setup_partial_root;
    });

    auto* contentRow = new wxBoxSizer(wxHORIZONTAL);
    auto* leftColumn = new wxBoxSizer(wxVERTICAL);
    auto* rightColumn = new wxBoxSizer(wxVERTICAL);

    leftColumn->Add(rootType, 0, wxALL | wxEXPAND, 5);
    leftColumn->Add(setupMode, 0, wxALL | wxEXPAND, 5);

    rightColumn->Add(optionSafemode, 0, wxALL, 5);
    rightColumn->Add(optionVerbose, 0, wxALL, 5);
    rightColumn->Add(optionRevert, 0, wxALL, 5);
    rightColumn->Add(optionDarkBlockchain, 0, wxALL, 5);
    rightColumn->AddStretchSpacer();

    contentRow->Add(leftColumn, 1, wxEXPAND | wxRIGHT, 10);
    contentRow->Add(rightColumn, 1, wxEXPAND | wxLEFT, 10);

    left->Add(contentRow, 1, wxEXPAND | wxLEFT | wxRIGHT, 10);
    left->Add(new wxStaticText(this, wxID_ANY, "Boot Arguments:"), 0, wxALL, 15);
    left->Add(bootArgs, 0, wxLEFT | wxRIGHT | wxEXPAND, 30);
    left->AddStretchSpacer();

    auto* back = new wxButton(this, wxID_ANY, "Back");
    right->Add(logo, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP | wxLEFT | wxRIGHT, 10);
    right->Add(logoText, 0, wxALIGN_CENTER_HORIZONTAL | wxTOP, 5);
    right->AddStretchSpacer();
    right->Add(back, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);

    back->Bind(wxEVT_BUTTON, [frame](wxCommandEvent&)
    {
        frame->ShowMain();
    });

    root->Add(left, 1, wxEXPAND);
    root->Add(new wxStaticLine(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_VERTICAL), 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
    root->Add(right, 0, wxEXPAND);

    SetSizer(root);
}

#endif // WITH_GUI
