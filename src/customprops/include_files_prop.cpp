/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for Include Files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "include_files_prop.h"

#include "include_files_dlg.h"

#include "../nodes/node_prop.h"  // NodeProperty class

IncludeFilesProperty::IncludeFilesProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString())
{
}

bool IncludeFilesDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    IncludeFilesDialog dlg(propGrid->GetPanel());
    dlg.Initialize(m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
