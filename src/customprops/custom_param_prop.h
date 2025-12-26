/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for custom control parameters
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "node_prop.h"  // NodeProperty class
#include "wxui/grid_property_dlg.h"

class Node;
class NodeEvent;

class EditParamsDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditParamsDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                      wxPGProperty* /* property unused */) override;

private:
    NodeProperty* m_prop;
};

class EditParamProperty : public wxStringProperty
{
public:
    EditParamProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new EditParamsDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

class EditParamsDialog : public GridPropertyDlgBase
{
public:
    EditParamsDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

    struct string_pairs
    {
        std::string style;
        std::string width;
    };

protected:
    void OnInit(wxInitDialogEvent& /* event unused */) override;
    void OnOK(wxCommandEvent& event) override;
    void OnCancel(wxCommandEvent& event) override { event.Skip(); }

    void OnNewRow(wxCommandEvent& /* event unused */) override;
    void OnDeleteRow(wxCommandEvent& /* event unused */) override;
    void OnUndoDelete(wxCommandEvent& /* event unused */) override;
    void OnUpdateUI(wxUpdateUIEvent& /* event unused */) override;

private:
    std::vector<wxue::string> m_fields;
    NodeProperty* m_prop { nullptr };

    wxString m_value;
    wxString m_deleted_col_0;
};
