/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Property editor for pop_custom_mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "edit_custom_mockup_base.h"

class Node;
class NodeEvent;
class NodeProperty;

class EditCustomMockupDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditCustomMockupDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class EditCustomMockupProperty : public wxStringProperty
{
public:
    EditCustomMockupProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new EditCustomMockupDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};

class EditCustomMockupDialog : public EditCustomMockupBase
{
public:
    EditCustomMockupDialog(wxWindow* parent, NodeProperty* prop);

protected:
};
