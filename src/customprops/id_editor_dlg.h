/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing a ID name and optional value
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-03-2026]

#pragma once

#include "id_editor_dlg_base.h"

#include <set>

class Node;

class IDEditorDlg : public IDEditorDlgBase
{
public:
    IDEditorDlg() {}  // If you use this constructor, you must call Create(parent)
    IDEditorDlg(wxWindow* parent);

    wxString GetResults() { return m_result; }

    void SetNode(Node* node) { m_node = node; }

    bool SelectPrefixSuffix(Node* node);

protected:
    // Handlers for IDEditorDlgBase events
    void OnAffirmative(wxUpdateUIEvent& event) override;
    void OnComboSelect(wxCommandEvent& event) override;
    void OnCustomID(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnMakeUnique(wxCommandEvent& event);
    void OnStandardID(wxCommandEvent& event) override;
    void OnStdChange(wxCommandEvent& event) override;

private:
    wxString BuildCompleteId() const;
    wxString BuildIdName(const wxString& id_part) const;
    void CollectFormIds();
    void UpdateDuplicateIdWarning();
    std::set<wxString> m_form_ids;
};
