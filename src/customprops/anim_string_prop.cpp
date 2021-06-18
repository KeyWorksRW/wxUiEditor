/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling animation files
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ttcwd.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "anim_string_prop.h"

#include "mainapp.h"       // MoveDirection -- Main application class
#include "node.h"          // Node -- Node class
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

bool AnimDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    ttlib::cwd cwd(true);
    if (wxGetApp().GetProject()->HasValue(prop_converted_art))
    {
        ttlib::ChangeDir(wxGetApp().GetProjectPtr()->prop_as_string(prop_converted_art));
        cwd.assignCwd();
    }

    wxFileDialog dlg(propGrid->GetPanel(), _tt("Open Header file"), cwd.wx_str(), wxEmptyString,
                     "Header|*.h;*.hpp;*.hh;*.hxx", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK)
    {
        ttString path = dlg.GetPath();
        path.make_relative(cwd);
        SetValue(path);
        return true;
    }
    return false;
}
