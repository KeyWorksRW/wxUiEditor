/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-21-2026]

// REVIEW: [Randalphwa - 08-21-2026] This appears to be dead code -- nobody is using it...

#include <tuple>  // for std::ignore

#include <wx/button.h>    // wxButton
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "generate_xrc_dlg.h"

#include "project_handler.h"  // ProjectHandler class

// If this constructor is used, the caller must call Create(parent)
GenerateXrcDlg::GenerateXrcDlg() {}

GenerateXrcDlg::GenerateXrcDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

wxButton* GenerateXrcDlg::GetSaveButton()
{
    if (!m_save_btn)
    {
        m_save_btn = wxStaticCast(FindWindowById(wxID_SAVE), wxButton);
    }
    return m_save_btn;
}

void GenerateXrcDlg::OnInit(wxInitDialogEvent& event)
{
    if (Project.HasValue(prop_combined_xrc_file))
    {
        m_filename = Project.as_string(prop_combined_xrc_file).wx();
        m_filePicker->SetPath(m_filename);
    }
    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (auto& form: forms)
    {
        if (form->HasValue(prop_xrc_file))
        {
            m_listbox->AppendString(wxue::string(form->as_string(prop_xrc_file))
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

void GenerateXrcDlg::OnCombinedFile(wxCommandEvent& /* event unused */)
{
    if (m_radio_combined->GetValue())
    {
        m_radio_separate->SetValue(false);
        m_separate_box->GetStaticBox()->Enable(false);
        m_combined_box->GetStaticBox()->Enable(true);

        if (wxButton* save_btn = GetSaveButton(); save_btn)
        {
            save_btn->Enable(!m_filename.empty());
        }
    }
}

void GenerateXrcDlg::OnSeparateFiles(wxCommandEvent& /* event unused */)
{
    if (m_radio_separate->GetValue())
    {
        m_radio_combined->SetValue(false);
        m_combined_box->GetStaticBox()->Enable(false);
        m_separate_box->GetStaticBox()->Enable(true);

        if (wxButton* save_btn = GetSaveButton(); save_btn)
        {
            save_btn->Enable(m_listbox->GetCount() > 0);
        }
    }
}

void GenerateXrcDlg::OnCombinedFilenameChanged(wxFileDirPickerEvent& /* event unused */)
{
    m_filename = m_filePicker->GetPath();
    if (wxButton* save_btn = GetSaveButton(); save_btn)
    {
        save_btn->Enable(!m_filename.empty());
    }
}

void GenerateXrcDlg::OnSave(wxCommandEvent& event)
{
    if (!Validate() || !TransferDataFromWindow())
    {
        return;
    }

    if (IsModal())
    {
        EndModal(wxID_OK);
    }

    SetReturnCode(wxID_OK);
    Show(false);

    event.Skip();  // This must be called for wxPersistenceManager to work
}
