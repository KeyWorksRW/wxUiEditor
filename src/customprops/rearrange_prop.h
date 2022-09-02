/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRearangeList contents editor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "node_prop.h"  // NodeProperty class
#include "wxui/grid_property_dlg.h"

class Node;
class NodeEvent;

class RearrangeDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    RearrangeDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class RearrangeProperty : public wxStringProperty
{
public:
    RearrangeProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new RearrangeDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};

class RearrangeDialog : public GridPropertyDlg
{
public:
    RearrangeDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

    struct string_pairs
    {
        std::string style;
        std::string width;
    };

protected:
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& event) override;
    void OnCancel(wxCommandEvent& event) override;

    void OnNewRow(wxCommandEvent& WXUNUSED(event)) override;
    void OnDeleteRow(wxCommandEvent& WXUNUSED(event)) override;
    void OnUndoDelete(wxCommandEvent& WXUNUSED(event)) override;
    void OnUpdateUI(wxUpdateUIEvent& WXUNUSED(event)) override;

private:
    std::vector<NODEPROP_CHECKLIST_ITEM> m_contents;
    NodeProperty* m_prop { nullptr };

    int m_label_width { 0 };

    wxString m_value;
    wxString m_deleted_col_0;
    wxString m_deleted_col_1;
};
