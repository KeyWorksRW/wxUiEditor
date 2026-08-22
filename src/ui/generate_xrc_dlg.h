/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

// clang-format off

#pragma once

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "generate_xrc_dlg_base.h"

// clang-format on

class wxButton;

class GenerateXrcDlg : public GenerateXrcDlgBase
{
public:
    GenerateXrcDlg();  // If you use this constructor, you must call Create(parent)
    GenerateXrcDlg(wxWindow* parent);

    wxue::string GetCombinedFilename() { return m_filename.utf8_string(); }
    bool CombineFiles() { return m_create_combined; }

protected:
    // Handlers for GenerateXrcDlgBase events

    void OnCombinedFile(wxCommandEvent& event) override;
    void OnCombinedFilenameChanged(wxFileDirPickerEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnSave(wxCommandEvent& event) override;
    void OnSeparateFiles(wxCommandEvent& event) override;

private:
    wxButton* GetSaveButton();
    wxButton* m_save_btn { nullptr };

    wxString m_filename;
};
