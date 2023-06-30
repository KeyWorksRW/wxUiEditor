/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for choosing a directory
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dirdlg.h>             // wxDirDialog base class
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "directory_prop.h"

#include "node_prop.h"        // NodeProperty class
#include "project_handler.h"  // ProjectHandler class

DirectoryProperty::DirectoryProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

bool DirectoryDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* property)
{
    wxSize dlg_sz;
    wxPoint dlg_pos;

    if (wxPropertyGrid::IsSmallScreen())
    {
        dlg_sz = wxDefaultSize;
        dlg_pos = wxDefaultPosition;
    }
    else
    {
        dlg_sz.Set(300, 400);
        dlg_pos = propGrid->GetGoodEditorDialogPosition(property, dlg_sz);
    }

    tt_string path = Project.get_ProjectPath();
    path.append_filename(m_prop->as_string());
    path.make_absolute();

    // If the directory doesn't exist, then we need to reset it. Otherwise on Windows, the
    // dialog will be for the computer, requiring the user to drill down to where the project
    // file is.
    if (!path.dir_exists())
    {
        path = Project.get_ProjectPath();
    }

    wxDirDialog dlg(propGrid, "Choose a directory:", path.make_wxString(),
                    wxDD_DEFAULT_STYLE | wxDD_CHANGE_DIR | wxDD_DIR_MUST_EXIST, dlg_pos, dlg_sz);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetPath());
        return true;
    }
    return false;
}
