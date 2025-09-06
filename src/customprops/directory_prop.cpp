/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for choosing a directory
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
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

    wxFileName path;
    auto node = m_prop->getNode();
    if (node->is_Gen(gen_wxFilePickerCtrl))
    {
        if (m_prop->as_string().size())
        {
            path.AssignDir(m_prop->as_string());
        }
        else
        {
            path = *Project.get_wxFileName();
            path.SetFullName(wxEmptyString);  // clear the project filename
        }
    }
    else
    {
        path = *Project.get_wxFileName();
        path.SetFullName(wxEmptyString);  // clear the project filename
        if (m_prop->as_string().size())
        {
            wxFileName prop_path;
            prop_path.AssignDir(m_prop->as_string());
            for (auto& iter: prop_path.GetDirs())
            {
                path.AppendDir(iter);
            }
            path.MakeAbsolute();
        }
    }

    // If the directory doesn't exist, then we need to reset it. Otherwise on Windows, the
    // dialog will be for the computer, requiring the user to drill down to where the project
    // file is.
    if (!node->is_Gen(gen_wxFilePickerCtrl) && !path.DirExists())
    {
        path = *Project.get_wxFileName();
        path.SetFullName(wxEmptyString);  // clear the project filename
    }

    auto style = wxDD_DEFAULT_STYLE | wxDD_CHANGE_DIR;
    if (!node->is_Gen(gen_wxFilePickerCtrl))
    {
        style |= wxDD_DIR_MUST_EXIST;
    }

    wxDirDialog dlg(propGrid, wxDirSelectorPromptStr, path.GetPath(), style, dlg_pos, dlg_sz);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetPath());
        return true;
    }
    return false;
}
