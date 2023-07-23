/////////////////////////////////////////////////////////////////////////////
// Purpose:   Uses IDEditorDlg to edit a custom ID
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "id_editor_dlg.h"

#include "id_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

ID_Property::ID_Property(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

bool ID_DialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    IDEditorDlg dlg(wxGetFrame().getWindow());
    dlg.SetNode(m_prop->getNode());
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
