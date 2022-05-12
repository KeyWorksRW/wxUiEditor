/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for HTML
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "wxui/edit_html_dialog_base.h"  // EditHtmlDialogBase

class Node;
class NodeEvent;
class NodeProperty;

class EditHtmlDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditHtmlDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class EditHtmlProperty : public wxStringProperty
{
public:
    EditHtmlProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new EditHtmlDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};

class EditHtmlDialog : public EditHtmlDialogBase
{
public:
    EditHtmlDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& event) override;
    void OnTextChange(wxStyledTextEvent& event) override;

    wxString m_value;
};
