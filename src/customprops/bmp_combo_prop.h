/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for wxBitmapComboBox contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "node_prop.h"  // NodeProperty class
#include "wxui/grid_property_dlg.h"

class Node;

class BmpComboDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    BmpComboDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                      wxPGProperty* /* property unused */) override;

private:
    NodeProperty* m_prop;
};

class BmpComboProperty : public wxStringProperty
{
public:
    BmpComboProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new BmpComboDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

class BmpComboDialog : public GridPropertyDlgBase
{
public:
    BmpComboDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

    void OnCellLeftDClick(wxGridEvent& event);

protected:
    void OnInit(wxInitDialogEvent& /* event unused */) override;
    void OnOK(wxCommandEvent& event) override;
    void OnCancel(wxCommandEvent& event) override;

    void OnNewRow(wxCommandEvent& /* event unused */) override;
    void OnDeleteRow(wxCommandEvent& /* event unused */) override;
    void OnUndoDelete(wxCommandEvent& /* event unused */) override;
    void OnUpdateUI(wxUpdateUIEvent& /* event unused */) override;

private:
    std::vector<NODEPROP_BMP_COMBO_ITEM> m_items;
    NodeProperty* m_prop { nullptr };

    wxString m_value;
    wxString m_deleted_col_0;
    wxString m_deleted_col_1;
};
