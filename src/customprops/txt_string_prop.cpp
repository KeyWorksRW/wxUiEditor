/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for single-line text
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "txt_string_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class

#include "../ui/editstringdialog_base.h"  // auto-generated: ../ui/editstringdialog_base.cpp

EditStringProperty::EditStringProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

class EditStringDialog : public EditStringDialogBase
{
public:
    EditStringDialog(wxWindow* parent, NodeProperty* prop) : EditStringDialogBase(parent)
    {
        SetTitle(ttlib::cstr() << prop->DeclName() << " property editor");
        m_value = prop->as_wxString();
    };
};

bool EditStringDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    EditStringDialog dlg(propGrid->GetPanel(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
