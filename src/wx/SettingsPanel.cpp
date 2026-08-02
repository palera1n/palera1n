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

#include "SettingsPanel.hpp"

#include <wx/wx.h>
#include <wx/notebook.h>
#include <wx/mstream.h>
#include <wx/image.h>
#include <wx/bitmap.h>
#include <wx/statline.h>

#include "AppFrame.hpp"
#include "DevicePanel.hpp"
#include "../globals.h"
#include "../paleinfo.h"
#include "../gen/embedded/logo.h"

auto createTitle = [](wxWindow* parent, const wxString& text)
{
    auto* title = new wxStaticText(parent, wxID_ANY, text);

    wxFont font = title->GetFont();
    font.SetPointSize(font.GetPointSize() - 1);
    font.SetWeight(wxFONTWEIGHT_BOLD);

    title->SetFont(font);


    return title;
};

auto createDescription = [](wxWindow* parent, const wxString& text)
{
    auto* description = new wxStaticText(parent, wxID_ANY, text, wxDefaultPosition, wxDefaultSize, wxST_WRAP);

    wxFont font = description->GetFont();
    font.SetPointSize(font.GetPointSize() - 2);
    description->SetFont(font);

    return description;
};

SettingsPanel::SettingsPanel(MainFrame* frame, wxWindow* parent)
    : DevicePanel(frame, parent)
{
    auto* root = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    auto* right = new wxBoxSizer(wxVERTICAL);

    wxMemoryInputStream stream(embedded_logo_png, embedded_logo_png_len);
    wxImage img(stream, wxBITMAP_TYPE_PNG);
    wxBitmap bmp(img);

    auto* logo = new wxStaticBitmap(this, wxID_ANY, bmp);
    auto* logoText = new wxStaticText(this, wxID_ANY, "Options");

    left->Add(new wxStaticText(this, wxID_ANY,
        "You may set the following options. If you don't know what they mean you'll probably have no reason to set them.",
        wxDefaultPosition,
        wxDefaultSize,
        wxST_WRAP
    ), 0, wxTOP | wxLEFT | wxRIGHT, 10);

    auto* notebook = new wxNotebook(this, wxID_ANY);
    auto* generalPage = new wxPanel(notebook);
    auto* advancedPage = new wxPanel(notebook);

    //
    // Controls
    //
    auto* optionSafemode = new wxCheckBox(generalPage, wxID_ANY, "Safe Mode");
    auto* optionVerbose = new wxCheckBox(generalPage, wxID_ANY, "Verbose Boot");
    auto* optionRevert = new wxCheckBox(generalPage, wxID_ANY, "Restore System");
    auto* optionDarkBlockchain = new wxCheckBox(generalPage, wxID_ANY, "Dark Blockchain");

    auto* rootless = new wxRadioButton(generalPage, wxID_ANY, "Rootless", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    auto* rootful = new wxRadioButton(generalPage, wxID_ANY, "Rootful");

    auto* setupBoot = new wxRadioButton(generalPage, wxID_ANY, "Boot", wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
    auto* setupFakeFS = new wxRadioButton(generalPage, wxID_ANY, "Create fakeFS");
    auto* setupBindFS = new wxRadioButton(generalPage, wxID_ANY, "Create Partial fakeFS");

    auto* bootArgs = new wxTextCtrl(advancedPage, wxID_ANY);
    auto* optionForceEnableSSV = new wxCheckBox(advancedPage, wxID_ANY, "Force Enable SSV Flag");

    //
    // Load
    //
    optionSafemode->SetValue(palerain_flags & palerain_option_safemode);
    optionVerbose->SetValue(palerain_flags & palerain_option_verbose_boot);
    optionRevert->SetValue(palerain_flags & palerain_option_force_revert);
    optionDarkBlockchain->SetValue(palerain_flags & palerain_option_flower_chain);

    bootArgs->SetValue(wxString(boot_args, wxConvUTF8));
    optionForceEnableSSV->SetValue(palerain_flags & palerain_option_ssv);

    rootless->SetValue(!(palerain_flags & palerain_option_rootful));
    rootful->SetValue(palerain_flags & palerain_option_rootful);

    if (palerain_flags & palerain_option_setup_rootful)
        setupFakeFS->SetValue(true);
    else if (palerain_flags & palerain_option_setup_partial_root)
        setupBindFS->SetValue(true);
    else
        setupBoot->SetValue(true);

    bool isRootful = rootful->GetValue();

    setupBoot->Enable(true);
    setupFakeFS->Enable(isRootful);
    setupBindFS->Enable(isRootful);

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

        if (value.length() > (sizeof(boot_args) - 0x20))
            return;

        snprintf(boot_args, sizeof(boot_args),
                 "%s",
                 value.ToStdString().c_str());
    });

    optionForceEnableSSV->Bind(wxEVT_CHECKBOX, [&](wxCommandEvent& e)
    {
        if (e.IsChecked())
            palerain_flags |= palerain_option_ssv;
        else
            palerain_flags &= ~palerain_option_ssv;
    });

    rootless->Bind(wxEVT_RADIOBUTTON,
        [setupBoot, setupFakeFS, setupBindFS](wxCommandEvent&)
    {
        palerain_flags &= ~(palerain_option_rootless |
                            palerain_option_rootful |
                            palerain_option_setup_rootful |
                            palerain_option_setup_partial_root);

        palerain_flags |= palerain_option_rootless;

        setupBoot->SetValue(true);
        setupFakeFS->Enable(false);
        setupBindFS->Enable(false);
    });

    rootful->Bind(wxEVT_RADIOBUTTON,
        [setupFakeFS, setupBindFS](wxCommandEvent&)
    {
        palerain_flags &= ~(palerain_option_rootless |
                            palerain_option_rootful);

        palerain_flags |= palerain_option_rootful;

        setupFakeFS->Enable(true);
        setupBindFS->Enable(true);
    });

    setupBoot->Bind(wxEVT_RADIOBUTTON, [](wxCommandEvent&)
    {
        palerain_flags &= ~(palerain_option_setup_rootful |
                            palerain_option_setup_partial_root);
    });

    setupFakeFS->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent&)
    {
        palerain_flags &= ~palerain_option_setup_partial_root;
        palerain_flags |= palerain_option_setup_rootful;
    });

    setupBindFS->Bind(wxEVT_RADIOBUTTON, [&](wxCommandEvent&)
    {
        palerain_flags &= ~palerain_option_setup_rootful;
        palerain_flags |= palerain_option_setup_partial_root;
    });

    //
    // General
    //
    auto* generalSizer = new wxBoxSizer(wxHORIZONTAL);

    auto* generalLeft = new wxBoxSizer(wxVERTICAL);
    auto* generalRight = new wxBoxSizer(wxVERTICAL);

    generalLeft->Add(createTitle(generalPage, "Startup"), 0, wxBOTTOM, 3);

    generalLeft->Add(optionSafemode, 0, wxTOP, 7);
    generalLeft->Add(optionVerbose, 0, wxTOP , 7);
    generalLeft->Add(optionRevert, 0, wxTOP, 7);
    generalLeft->Add(optionDarkBlockchain, 0, wxTOP, 7);

    generalLeft->AddStretchSpacer();

    generalRight->Add(createTitle(generalPage, "Jailbreak Type"), 0, wxBOTTOM, 3);

    generalRight->Add(rootless, 0, wxTOP, 7);
    generalRight->Add(rootful, 0, wxTOP | wxBOTTOM, 5);

    generalRight->Add(createTitle(generalPage, "Startup (Rootful)"), 0, wxTOP | wxBOTTOM, 3);

    generalRight->Add(setupBoot, 0, wxTOP, 7);
    generalRight->Add(setupFakeFS, 0, wxTOP, 5);
    generalRight->Add(setupBindFS, 0, wxTOP, 5);

    generalRight->AddStretchSpacer();

    generalSizer->Add(generalLeft, 1, wxEXPAND | wxRIGHT | wxLEFT, 10);
    generalSizer->Add(generalRight, 1, wxEXPAND | wxLEFT, 10);

    generalPage->SetSizer(generalSizer);

    //
    // Advanced
    //
    auto* advancedOuterSizer = new wxBoxSizer(wxVERTICAL);
    auto* advancedSizer = new wxBoxSizer(wxVERTICAL);

    advancedSizer->Add(createTitle(advancedPage, "Boot Arguments"), 0, wxBOTTOM, 3);

    advancedSizer->Add(bootArgs, 0, wxTOP | wxBOTTOM | wxEXPAND, 7);

    advancedSizer->Add(createTitle(advancedPage, "Advanced"), 0, wxTOP | wxBOTTOM, 3);

    advancedSizer->Add(optionForceEnableSSV, 0, wxTOP | wxBOTTOM, 7);
    advancedSizer->Add(createDescription(advancedPage, "Forces the jailbreak to detect SSV as being required."), 0, wxLEFT | wxRIGHT, 20);

    advancedOuterSizer->Add(advancedSizer, 1, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 10);

    advancedPage->SetSizer(advancedOuterSizer);

    notebook->AddPage(generalPage, "General", true);
    notebook->AddPage(advancedPage, "Advanced");

    left->Add(notebook, 1, wxEXPAND | wxALL, 10);

    //
    // Right side
    //
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
