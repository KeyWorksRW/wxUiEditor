/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for generating XRC file(s)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../wxui/generate_xrc_dlg_base.h"

class Project;

class GenerateXrcDlg : public GenerateXrcDlgBase
{
public:
    GenerateXrcDlg();  // If you use this constructor, you must call Create(parent)
    GenerateXrcDlg(wxWindow* parent);

    ttlib::cstr GetCombinedFilename() { return m_filename.utf8_string(); }
    bool CombineFiles() { return m_create_combined; }

protected:
    // Handlers for GenerateXrcDlgBase events
    void OnCombinedFile(wxCommandEvent& WXUNUSED(event)) override;
    void OnCombinedFilenameChanged(wxFileDirPickerEvent& WXUNUSED(event)) override;
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnSeparateFiles(wxCommandEvent& WXUNUSED(event)) override;
    void OnSave(wxCommandEvent& event) override;

private:
    wxString m_filename;
};
