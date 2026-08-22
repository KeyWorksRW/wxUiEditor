/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing a list of include files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "include_files_dlg_base.h"

class IncludeFilesDialog : public IncludeFilesDialogBase
{
public:
    IncludeFilesDialog();  // If you use this constructor, you must call Create(parent)
    IncludeFilesDialog(wxWindow* parent);

    const wxString& GetResults() { return m_value; }
    void Initialize(NodeProperty* prop);
    void SetButtonsEnableState(bool set_ok_btn = true);

protected:
    // Handlers for IncludeFilesDialogBase events
    void OnAdd(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnItemSelected(wxCommandEvent& event) override;
    void OnMoveDown(wxCommandEvent& event) override;
    void OnMoveUp(wxCommandEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnRemove(wxCommandEvent& event) override;
    void OnSort(wxCommandEvent& event) override;

private:
    wxString m_value;

    NodeProperty* m_prop { nullptr };

    GenLang m_language = GenLang::cplusplus;
};
