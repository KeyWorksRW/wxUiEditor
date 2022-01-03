/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for font property
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "font_string_prop.h"

#include "font_prop_dlg.h"  // FontPropDlg -- Dialog for editing Font Property

#include "../nodes/node_prop.h"  // NodeProperty class

FontStringProperty::FontStringProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

bool FontStringDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    FontPropDlg dlg(propGrid->GetPanel(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
