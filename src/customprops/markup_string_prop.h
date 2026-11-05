/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for labels that support wxWidgets markup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "../wxui/markstring_dlg_base.h"  // MarkStringDlgBase

class NodeProperty;

class MarkupDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    MarkupDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* /*propGrid unused*/,
                      wxPGProperty* /*property unused*/) override;

private:
    NodeProperty* m_prop;
};

class MarkupStringProperty : public wxStringProperty
{
public:
    MarkupStringProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new MarkupDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};

// All controls and event bindings come from the generated base class; this class only supplies the
// property-specific behaviour for them.
class MarkStringDlg : public MarkStringDlgBase
{
public:
    MarkStringDlg(wxWindow* parent, NodeProperty* prop);

    wxString GetResults() const { return m_result; }

protected:
    void OnMarkupChanged(wxCommandEvent& event) override;
    void OnInsertTag(wxCommandEvent& event) override;
    void OnInsertColour(wxCommandEvent& event) override;
    void OnOK(wxCommandEvent& event) override;

private:
    void UpdatePreview();
    void InsertMarkup(const wxString& open_tag, const wxString& close_tag);

    NodeProperty* m_prop { nullptr };
    wxString m_result;
};
