/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for Growable Sizer Columns
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "node_prop.h"  // NodeProperty class
#include "wxui/grid_property_dlg.h"

class Node;
class NodeEvent;

class GrowColumnsDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    GrowColumnsDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid),
                      wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class GrowColumnsProperty : public wxStringProperty
{
public:
    GrowColumnsProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new GrowColumnsDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

class GrowColumnsDialog : public GridPropertyDlgBase
{
public:
    GrowColumnsDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& event) override;

    void OnNewRow(wxCommandEvent& WXUNUSED(event)) override;
    void OnDeleteRow(wxCommandEvent& WXUNUSED(event)) override;
    void OnUpdateUI(wxUpdateUIEvent& WXUNUSED(event)) override;

    void OnCancel(wxCommandEvent& event) override { event.Skip(); }
    void OnUndoDelete(wxCommandEvent& event) override { event.Skip(); }

private:
    struct GrowColumnsEntry
    {
        int column;
        int proportion;
    };

    std::vector<GrowColumnsEntry> m_grow_columns;
    NodeProperty* m_prop { nullptr };

    wxString m_value;
};
