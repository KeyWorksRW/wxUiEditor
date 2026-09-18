/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node search dialog implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/node_search_dlg_base.h"

class NodeSearchDlg : public NodeSearchDlgBase
{
public:
    NodeSearchDlg();  // If you use this constructor, you must call Create(parent)
    NodeSearchDlg(wxWindow* parent);

    std::string& GetNameChoice() { return m_name; }

    Node* GetForm() { return m_form; }

    void FindGenerators(Node* node);

    void FindVariables(Node* node);

    void FindLabels(Node* node);

protected:
    // Handlers for NodeSearchDlgBase events

    void OnGenerators(wxCommandEvent& event) override;
    void OnIDs(wxCommandEvent& event) override;
    void OnInit(wxInitDialogEvent& event) override;
    void OnLabels(wxCommandEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnSearchText(wxCommandEvent& event) override;
    void OnSelectLocated(wxCommandEvent& event) override;
    void OnUnused(wxCommandEvent& event) override;
    void OnVariables(wxCommandEvent& event) override;

private:
    std::string m_name;  // could be gen_name, var_name, label or ID

    Node* m_form = nullptr;

    std::map<std::string, std::set<Node*>> m_map_found;
};

// Find a form by its class_name property, searching recursively through folders
[[nodiscard]] auto FindNodeByClassName(Node* node_start, const std::string& classname) -> Node*;
