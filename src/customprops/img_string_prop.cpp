/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "ttcwd.h"  // cwd -- Class for storing and optionally restoring the current directory

#include "img_string_prop.h"

#include "art_prop_dlg.h"  // ArtBrowserDialog -- Art Property Dialog for image property
#include "mainapp.h"       // MoveDirection -- Main application class
#include "node.h"          // Node -- Node class
#include "uifuncs.h"       // Miscellaneous functions for displaying UI

bool ImageDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    if (m_img_props.type.contains("Art"))
    {
        ArtBrowserDialog dlg(propGrid->GetPanel(), m_img_props);
        if (dlg.ShowModal() == wxID_OK)
        {
            SetValue(dlg.GetResults());
            return true;
        }
        return false;
    }
    else if (m_img_props.type.contains("XPM") || m_img_props.type.contains("Header"))
    {
        ttlib::cwd cwd(true);
        if (wxGetApp().GetProject()->HasValue(prop_converted_art))
        {
            ttlib::ChangeDir(wxGetApp().GetProjectPtr()->prop_as_string(prop_converted_art));
            cwd.assignCwd();
        }

        ttlib::cstr pattern = m_img_props.type.contains("XPM") ? "XPM File (*.xpm)|*.xpm" : "Header|*.h;*.hpp;*.hh;*.hxx";
        wxFileDialog dlg(propGrid->GetPanel(), _tt("Open XPM file"), cwd.wx_str(), wxEmptyString, pattern,
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            ttString path = dlg.GetPath();
            path.make_relative(cwd);
            SetValue(path);
            return true;
        }
        return false;
    }

    return false;
}
