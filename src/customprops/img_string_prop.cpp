/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "img_string_prop.h"

#include "art_prop_dlg.h"  // ArtBrowserDialog -- Art Property Dialog for image property
#include "mainapp.h"       // MoveDirection -- Main application class
#include "node.h"          // Node -- Node class
#include "utils.h"         // Utility functions that work with properties

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
    else if (m_img_props.type.contains("Embed"))
    {
        ttSaveCwd cwd;
        if (wxGetApp().GetProject()->HasValue(prop_original_art))
        {
            auto dir = wxGetApp().GetOriginalArtDir();
            if (dir.dir_exists())
            {
                wxFileName::SetCwd(dir);
            }
        }

        wxString pattern;
        if (m_img_props.IsAnimationType())
        {
            pattern = "All files|*.*|Gif|*.gif|Ani|*.ani||";
        }
        else
        {
            pattern = "All files|*.*|PNG|*.png|XPM|*.xpm|Tiff|*.tif;*.tiff|Bitmaps|*.bmp|Icon|*.ico||";
        }

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString, pattern,
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            ttString name(dlg.GetPath());
            name.make_relative_wx(wxGetApp().GetProjectPath());
            name.backslashestoforward();
            SetValue(name);
            return true;
        }
        return false;
    }
    else if (m_img_props.type.contains("XPM") || m_img_props.type.contains("Header"))
    {
        ttSaveCwd cwd;
        if (wxGetApp().GetProject()->HasValue(prop_original_art) && wxGetApp().GetOriginalArtDir().dir_exists())
        {
            wxFileName::SetCwd(wxGetApp().GetOriginalArtDir());
        }

        wxString pattern;
        if (m_img_props.IsAnimationType())
        {
            pattern = "Header files (*.h_img)|*.h_img";
        }
        else
        {
            pattern = m_img_props.type.contains("XPM") ? "XPM files (*.xpm)|*.xpm" : "Header files (*.h_img)|*.h_img";
        }

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString, pattern,
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            ttString name(dlg.GetPath());
            name.make_relative_wx(wxGetApp().GetProjectPath());
            name.backslashestoforward();
            SetValue(name);
            return true;
        }
        return false;
    }

    return false;
}
