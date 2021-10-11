/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for Importing a Windows resource file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "importwinres_base.h"

class ImportWinResDlg : public ImportWinResBase
{
public:
    ImportWinResDlg(wxWindow* parent, ttString filename = wxEmptyString);

    const ttlib::cstr& GetRcFilename() { return m_rcFilename; }
    std::vector<ttlib::cstr>& GetDlgNames() { return m_dialogs; }

protected:
    // Handlers for ImportWinResBase events.
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnResourceFile(wxFileDirPickerEvent& WXUNUSED(event)) override;
    void OnSelectAll(wxCommandEvent& WXUNUSED(event)) override;
    void OnClearAll(wxCommandEvent& WXUNUSED(event)) override;
    void OnOk(wxCommandEvent& WXUNUSED(event)) override;

    void ReadRcFile();

private:
    ttlib::cstr m_rcFilename;
    std::vector<ttlib::cstr> m_dialogs;
};
