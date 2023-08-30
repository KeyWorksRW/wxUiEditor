///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/persist.h>
#include <wx/persist/toplevel.h>
#include <wx/stattext.h>
#include <wx/valgen.h>

#include "generate_xrc_dlg.h"

bool GenerateXrcDlg::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* dlg_sizer = new wxBoxSizer(wxVERTICAL);

    m_radio_combined = new wxRadioButton(this, wxID_ANY, "Combined File");
    m_radio_combined->SetValidator(wxGenericValidator(&m_create_combined));

    m_combined_box = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, m_radio_combined), wxVERTICAL);

    m_filePicker = new wxFilePickerCtrl(m_combined_box->GetStaticBox(), wxID_ANY, wxEmptyString, "Combined XRC File", "*.xrc",
        wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL|wxFLP_SAVE);
    m_filePicker->SetMinSize(ConvertDialogToPixels(wxSize(120, -1)));
    m_combined_box->Add(m_filePicker, wxSizerFlags(1).Expand().Border(wxALL));

    dlg_sizer->Add(m_combined_box, wxSizerFlags().Expand().Border(wxALL));

    m_radio_separate = new wxRadioButton(this, wxID_ANY, "Separate Files");
    m_separate_box = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, m_radio_separate), wxVERTICAL);

    auto* staticText = new wxStaticText(m_separate_box->GetStaticBox(), wxID_ANY,
        "Forms not listed do not have xrc_file set.");
    m_separate_box->Add(staticText, wxSizerFlags().Border(wxALL));

    m_listbox = new wxListBox(m_separate_box->GetStaticBox(), wxID_ANY);
    m_listbox->SetMinSize(ConvertDialogToPixels(wxSize(-1, 50)));
    m_separate_box->Add(m_listbox, wxSizerFlags(1).Expand().Border(wxALL));

    dlg_sizer->Add(m_separate_box, wxSizerFlags().Expand().Border(wxALL));

    auto* stdBtn = new wxStdDialogButtonSizer();
    stdBtn->AddButton(new wxButton(this, wxID_SAVE));
    stdBtn->AddButton(new wxButton(this, wxID_CANCEL));
    stdBtn->Realize();
    dlg_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizerAndFit(dlg_sizer);
    Centre(wxBOTH);

    wxPersistentRegisterAndRestore(this, "GenerateXrcDlg");

    // Event handlers
    Bind(wxEVT_BUTTON, &GenerateXrcDlg::OnSave, this, wxID_SAVE);
    m_filePicker->Bind(wxEVT_FILEPICKER_CHANGED, &GenerateXrcDlg::OnCombinedFilenameChanged, this);
    Bind(wxEVT_INIT_DIALOG, &GenerateXrcDlg::OnInit, this);
    m_radio_combined->Bind(wxEVT_RADIOBUTTON, &GenerateXrcDlg::OnCombinedFile, this);
    m_radio_separate->Bind(wxEVT_RADIOBUTTON, &GenerateXrcDlg::OnSeparateFiles, this);

    return true;
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************

/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "project_handler.h"  // ProjectHandler class

void GenerateXrcDlg::OnInit(wxInitDialogEvent& event)
{
    if (Project.hasValue(prop_combined_xrc_file))
    {
        m_filename = Project.as_string(prop_combined_xrc_file).make_wxString();
        m_filePicker->SetPath(m_filename);
    }
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (auto& form: forms)
    {
        if (form->hasValue(prop_xrc_file))
        {
            m_listbox->AppendString(tt_string(form->as_string(prop_xrc_file))
                                    << '(' << form->as_string(prop_class_name) << ')');
        }
    }

    m_create_combined = Project.as_bool(prop_combine_all_forms);
    wxCommandEvent dummy;
    if (m_create_combined)
    {
        m_radio_combined->SetValue(true);
        OnCombinedFile(dummy);
    }
    else
    {
        m_radio_separate->SetValue(true);
        OnSeparateFiles(dummy);
    }

    event.Skip();  // transfer all validator data to their windows and update UI
}

void GenerateXrcDlg::OnCombinedFile(wxCommandEvent& WXUNUSED(event))
{
    if (m_radio_combined->GetValue())
    {
        m_radio_separate->SetValue(false);
        m_separate_box->GetStaticBox()->Enable(false);
        m_combined_box->GetStaticBox()->Enable(true);

        auto btn = FindWindowById(wxID_SAVE);
        if (btn)
            btn->Enable(m_filename.size() > 0);
    }
}

void GenerateXrcDlg::OnSeparateFiles(wxCommandEvent& WXUNUSED(event))
{
    if (m_radio_combined->GetValue())
    {
        m_radio_combined->SetValue(false);
        m_combined_box->GetStaticBox()->Enable(false);
        m_separate_box->GetStaticBox()->Enable(true);

        auto btn = FindWindowById(wxID_SAVE);
        if (btn)
            btn->Enable(m_listbox->GetCount() > 0);
    }
}

void GenerateXrcDlg::OnCombinedFilenameChanged(wxFileDirPickerEvent& WXUNUSED(event))
{
    m_filename = m_filePicker->GetPath();
    auto btn = FindWindowById(wxID_SAVE);
    if (btn)
        btn->Enable(m_filename.size() > 0);
}

void GenerateXrcDlg::OnSave(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
        return;

    if (IsModal())
        EndModal(wxID_OK);
    else
    {
        SetReturnCode(wxID_OK);
        Show(false);
    }

    event.Skip();  // This must be called for wxPersistenceManager to work
}
