/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for Growable Sizer Rows
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

class GrowRowsDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    GrowRowsDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid),
                      wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class GrowRowsProperty : public wxStringProperty
{
public:
    GrowRowsProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new GrowRowsDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

class GrowRowsDialog : public GridPropertyDlgBase
{
public:
    GrowRowsDialog(wxWindow* parent, NodeProperty* prop);
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
    struct GrowRowsEntry
    {
        int index;
        int proportion;
    };

    std::vector<GrowRowsEntry> m_grow_entries;
    NodeProperty* m_prop { nullptr };

    wxString m_value;
};
