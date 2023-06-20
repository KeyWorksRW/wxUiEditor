/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for Include Files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

class Node;
class NodeEvent;
class NodeProperty;

class IncludeFilesDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    IncludeFilesDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop = nullptr;
};

class IncludeFilesProperty : public wxStringProperty
{
public:
    IncludeFilesProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new IncludeFilesDialogAdapter(m_prop); }

private:
    NodeProperty* m_prop = nullptr;
};
