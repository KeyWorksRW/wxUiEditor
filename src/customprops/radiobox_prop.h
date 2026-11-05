/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for wxRadioBox items
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "../wxui/radiobox_dlg_base.h"
#include "node_prop.h"  // NodeProperty class

class RadioBoxDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    RadioBoxDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                      wxPGProperty* /* property unused */) override;

private:
    NodeProperty* m_prop;
};

class RadioBoxProperty : public wxStringProperty
{
public:
    RadioBoxProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new RadioBoxDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

class RadioBoxDlg : public RadioBoxDlgBase
{
public:
    RadioBoxDlg(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    void OnInit(wxInitDialogEvent& /* event unused */) override;
    void OnOK(wxCommandEvent& event) override;
    void OnCancel(wxCommandEvent& event) override;

    void OnNewRow(wxCommandEvent& /* event unused */) override;
    void OnDeleteRow(wxCommandEvent& /* event unused */) override;
    void OnUndoDelete(wxCommandEvent& /* event unused */) override;
    void OnUpdateUI(wxUpdateUIEvent& /* event unused */) override;

private:
    std::vector<NODEPROP_RADIOBOX_ITEM> m_items;
    NodeProperty* m_prop { nullptr };

    wxString m_value;
    wxString m_deleted_label;
    wxString m_deleted_enabled;
    wxString m_deleted_show;
    wxString m_deleted_tooltip;
    wxString m_deleted_helptext;
};
