/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "editcodedialog_base.h"

class Node;
class NodeEvent;
class NodeProperty;

class EditCodeDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditCodeDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class EditCodeProperty : public wxStringProperty
{
public:
    EditCodeProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new EditCodeDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};

class EditCodeDialog : public EditCodeDialogBase
{
public:
    EditCodeDialog(wxWindow* parent, NodeProperty* prop);
    const wxString& GetResults() { return m_value; }

protected:
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& event) override;

    wxString m_value;
};
