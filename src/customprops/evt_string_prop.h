/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

class NodeEvent;

class EventStringDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EventStringDialogAdapter(NodeEvent* event) : wxPGEditorDialogAdapter(), m_event(event) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeEvent* m_event;
};

class EventStringProperty : public wxStringProperty
{
public:
    EventStringProperty(const wxString& label, NodeEvent* event);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new EventStringDialogAdapter(m_event); }

private:
    NodeEvent* m_event;
};
