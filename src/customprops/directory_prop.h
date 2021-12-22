/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for choosing a directory
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

class Node;
class NodeProperty;

class DirectoryDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    DirectoryDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class DirectoryProperty : public wxStringProperty
{
public:
    DirectoryProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new DirectoryDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};
