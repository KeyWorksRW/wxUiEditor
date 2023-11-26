/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Property editor for pop_custom_mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

#include "edit_custom_mockup.h"

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;

EditCustomMockupProperty::EditCustomMockupProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

EditCustomMockupDialog::EditCustomMockupDialog(wxWindow* parent, NodeProperty* prop) : EditCustomMockupBase(parent)
{
    m_result = prop->as_wxString();
};

bool EditCustomMockupDialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    EditCustomMockupDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
