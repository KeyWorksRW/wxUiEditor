/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "img_string_prop.h"

#include "art_prop_dlg.h"   // ArtBrowserDialog -- Art Property Dialog for image property
#include "node.h"           // Node -- Node class
#include "project_class.h"  // Project class
#include "utils.h"          // Utility functions that work with properties

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
        if (GetProject()->HasValue(prop_art_directory))
        {
            auto dir = GetProject()->GetArtDirectory();
            if (dir.dir_exists())
            {
                wxFileName::SetCwd(dir);
            }
        }

        wxString pattern;
        if (m_img_props.IsAnimationType())
        {
            pattern = "Animation files|*.*|Gif|*.gif|Ani|*.ani||";
        }
        else
        {
            pattern =
                "Bitmap files|*.png;*.bmp;*.ico;*.xpm|PNG|*.png|XPM|*.xpm|Tiff|*.tif;*.tiff|Bitmaps|*.bmp|Icon|*.ico||";
        }

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString, pattern,
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            ttString name(dlg.GetPath());
            name.make_relative_wx(GetProject()->GetProjectPath());
            name.backslashestoforward();
            SetValue(name);
            return true;
        }
        return false;
    }
    else if (m_img_props.type.contains("XPM") || m_img_props.type.contains("SVG"))
    {
        ttSaveCwd cwd;
        if (GetProject()->HasValue(prop_art_directory) && GetProject()->GetArtDirectory().dir_exists())
        {
            wxFileName::SetCwd(GetProject()->GetArtDirectory());
        }

        wxString pattern;
        if (m_img_props.type.contains("SVG"))
        {
            pattern = "SVG files (*.svg)|*.svg";
        }
        else
        {
            pattern = "XPM files (*.xpm)|*.xpm";
        }

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString, pattern,
                         wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            ttString name(dlg.GetPath());
            name.make_relative_wx(GetProject()->GetProjectPath());
            name.backslashestoforward();
            SetValue(name);
            return true;
        }
        return false;
    }

    return false;
}
