///////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for adding system header files to a list
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
///////////////////////////////////////////////////////////////////////////////
// CR: [08-20-2026]

#pragma once

#include <string>
#include <vector>

#include "sys_header_dlg_base.h"

class SysHeaderDlg : public SysHeaderDlgBase
{
public:
    SysHeaderDlg();  // If you use this constructor, you must call Create(parent)
    SysHeaderDlg(wxWindow* parent);

    void Initialize(NodeProperty* prop);

    const wxue::string& GetResults() const { return m_value; }

    void SetButtonsEnableState(bool set_ok_btn = true);

protected:
    // Handlers for SysHeaderDlgBase events

    void OnDirectory(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnRootSelected(wxCommandEvent& event) override;

private:
    // Class member variables

    wxue::string m_value;

    // Header files pre-checked from the property value; matched by name on each root reload
    std::vector<std::string> m_prechecked;

    wxArrayInt m_checked_indices;  // scratch buffer for SetButtonsEnableState

    NodeProperty* m_prop { nullptr };

    GenLang m_language = GenLang::cplusplus;

    wxFileHistory m_FileHistory;
};
