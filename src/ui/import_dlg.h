/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to import one or more projects
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/filehistory.h>  // wxFileHistory class

#include "import_base.h"

#include "wxue_namespace/wxue_string.h"  // wxue::string

class ImportDlg : public ImportBase
{
public:
    ImportDlg(wxWindow* parent = nullptr);

    [[nodiscard]] auto isImportFormBuilder() -> bool { return m_radio_wxFormBuilder->GetValue(); }
    [[nodiscard]] auto isImportSmith() -> bool { return m_radio_wxSmith->GetValue(); }
    [[nodiscard]] auto isImportXRC() -> bool { return m_radio_XRC->GetValue(); }
    [[nodiscard]] auto isImportWinRes() -> bool { return m_radio_WindowsResource->GetValue(); }
    [[nodiscard]] auto isImportDialogBlocks() -> bool { return m_radio_DialogBlocks->GetValue(); }

    auto GetFileList() -> std::vector<wxue::string>& { return m_lstProjects; };

protected:
    void CheckResourceFiles(wxArrayString& files);

    void OnCheckFiles(wxCommandEvent& event) override;
    void OnCrafter(wxCommandEvent& event) override;
    void OnDirectory(wxCommandEvent& event) override;
    void OnFormBuilder(wxCommandEvent& event) override;
    void OnDialogBlocks(wxCommandEvent& event) override;
    void OnInitDialog(wxInitDialogEvent& event) override;
    void OnWxGlade(wxCommandEvent& event) override;

    void OnSelectAll(wxCommandEvent& event) override;
    void OnSelectNone(wxCommandEvent& event) override;
    void OnWindowsResource(wxCommandEvent& event) override;
    void OnWxSmith(wxCommandEvent& event) override;
    void OnXRC(wxCommandEvent& event) override;

    void OnOK(wxCommandEvent& event) override;

    void OnRecentDir(wxCommandEvent& event) override;
    void OnRemove(wxCommandEvent& event) override;

private:
    std::vector<wxue::string> m_lstProjects;
    wxFileHistory m_FileHistory;
};
