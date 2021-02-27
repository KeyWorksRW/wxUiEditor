/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to create a new project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "newproject_base.h"

class NewProjectDlg : public NewProjectBase
{
public:
    NewProjectDlg(wxWindow* parent = nullptr);

    bool isImportFormBuilder() { return m_radioBtnFormBuilder->GetValue(); }
    bool isImportSmith() { return m_radioBtnSmith->GetValue(); }
    bool isImportXRC() { return m_radioBtnXrc->GetValue(); }
    bool isImportWinRes() { return m_radioBtnWinRes->GetValue(); }

    std::vector<ttString>& GetFileList() { return m_lstProjects; };

protected:
    // void OnCheckFiles(wxCommandEvent& event) override;
    void OnDirectory(wxCommandEvent& event) override;
    void OnEmptyProject(wxCommandEvent& event) override;
    void OnFormBuilder(wxCommandEvent& event) override;
    void OnInitDialog(wxInitDialogEvent& event) override;

    void OnSelectAll(wxCommandEvent& event) override;
    void OnSelectNone(wxCommandEvent& event) override;
    void OnWindowsResource(wxCommandEvent& event) override;
    void OnWxSmith(wxCommandEvent& event) override;
    void OnXRC(wxCommandEvent& event) override;

    void OnOK(wxCommandEvent& event) override;

private:
    std::vector<ttString> m_lstProjects;
};
