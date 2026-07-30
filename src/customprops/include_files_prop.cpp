/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for Include Files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "include_files_prop.h"

#include "include_files_dlg.h"

#include "../nodes/node_prop.h"  // NodeProperty class

IncludeFilesProperty::IncludeFilesProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

bool IncludeFilesDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid,
                                             wxPGProperty* /* property unused */)
{
    IncludeFilesDialog include_files_dlg(propGrid->GetPanel());
    include_files_dlg.Initialize(m_prop);
    if (include_files_dlg.ShowModal() == wxID_OK)
    {
        SetValue(include_files_dlg.GetResults());
        return true;
    }

    return false;
}
