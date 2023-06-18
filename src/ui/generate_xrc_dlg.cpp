/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "generate_xrc_dlg.h"  // auto-generated: ../wxui/generate_xrc_dlg_base.h and ../wxui/generate_xrc_dlg_base.cpp

#include "project_handler.h"  // ProjectHandler class

// If this constructor is used, the caller must call Create(parent)
GenerateXrcDlg::GenerateXrcDlg() {}

GenerateXrcDlg::GenerateXrcDlg(wxWindow* parent)
{
    Create(parent);
}

void GenerateXrcDlg::OnInit(wxInitDialogEvent& event)
{
    if (Project.HasValue(prop_combined_xrc_file))
    {
        m_filename = Project.value(prop_combined_xrc_file).make_wxString();
        m_filePicker->SetPath(m_filename);
    }
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (auto& form: forms)
    {
        if (form->HasValue(prop_xrc_file))
        {
            m_listbox->AppendString(tt_string(form->value(prop_xrc_file)) << '(' << form->value(prop_class_name) << ')');
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
