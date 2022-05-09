/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to import one or more projects
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/filehistory.h>  // wxFileHistory class

#include "import_base.h"

#include "ttstr.h"  // ttString -- wxString with additional methods similar to ttlib::cstr

class ImportDlg : public ImportBase
{
public:
    ImportDlg(wxWindow* parent = nullptr);

    bool isImportFormBuilder() { return m_radio_wxFormBuilder->GetValue(); }
    bool isImportSmith() { return m_radio_wxSmith->GetValue(); }
    bool isImportXRC() { return m_radio_XRC->GetValue(); }
    bool isImportWinRes() { return m_radio_WindowsResource->GetValue(); }

    std::vector<ttString>& GetFileList() { return m_lstProjects; };

protected:
    void CheckResourceFiles(wxArrayString& files);

    void OnCheckFiles(wxCommandEvent& event) override;
    void OnCrafter(wxCommandEvent& event) override;
    void OnDirectory(wxCommandEvent& event) override;
    void OnFormBuilder(wxCommandEvent& event) override;
    void OnInitDialog(wxInitDialogEvent& event) override;
    void OnWxGlade(wxCommandEvent& event) override;

    void OnSelectAll(wxCommandEvent& event) override;
    void OnSelectNone(wxCommandEvent& event) override;
    void OnWindowsResource(wxCommandEvent& event) override;
    void OnWxSmith(wxCommandEvent& event) override;
    void OnXRC(wxCommandEvent& event) override;

    void OnOK(wxCommandEvent& event) override;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    void OnRecentDir(wxCommandEvent& event) override;
    void OnRemove(wxCommandEvent& event) override;
#endif

private:
    std::vector<ttString> m_lstProjects;
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    wxFileHistory m_FileHistory;
#endif
};
