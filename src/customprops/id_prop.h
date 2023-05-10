/////////////////////////////////////////////////////////////////////////////
// Purpose:   Uses IDEditorDlg to edit a custom ID
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

#include "node_prop.h"  // NodeProperty class
#include "wxui/grid_property_dlg.h"

class Node;
class NodeEvent;

class ID_DialogAdapter : public wxPGEditorDialogAdapter
{
public:
    ID_DialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class ID_Property : public wxStringProperty
{
public:
    ID_Property(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new ID_DialogAdapter(m_prop); }

private:
    NodeProperty* m_prop;
};
