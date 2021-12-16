/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for colour
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

class NodeProperty;

class EditColourDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditColourDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property)) override;

private:
    NodeProperty* m_prop;
};

class EditColourProperty : public wxStringProperty
{
public:
    EditColourProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override { return new EditColourDialogAdapter(m_prop); }

    void OnCustomPaint(wxDC& dc, const wxRect& rect, wxPGPaintData& paintdata) override;
    wxSize OnMeasureImage(int /* item */) const override { return wxPG_DEFAULT_IMAGE_SIZE; }

private:
    NodeProperty* m_prop;
};
