/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for single line code
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/propgrid/editors.h>  // wxPropertyGrid editors
#include <wx/propgrid/props.h>    // wxPropertyGrid Property Classes

class NodeProperty;

class EditCodeSingleDialogAdapter : public wxPGEditorDialogAdapter
{
public:
    EditCodeSingleDialogAdapter(NodeProperty* prop) : wxPGEditorDialogAdapter(), m_prop(prop) {}

    bool DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                      wxPGProperty* /* property unused */) override;

private:
    NodeProperty* m_prop;
};

class EditCodeSingleProperty : public wxStringProperty
{
public:
    EditCodeSingleProperty(const wxString& label, NodeProperty* prop);

    // Set editor to have button
    const wxPGEditor* DoGetEditorClass() const override { return wxPGEditor_TextCtrlAndButton; }

    // Set what happens on button click
    wxPGEditorDialogAdapter* GetEditorDialog() const override
    {
        return new EditCodeSingleDialogAdapter(m_prop);
    }

private:
    NodeProperty* m_prop;
};
