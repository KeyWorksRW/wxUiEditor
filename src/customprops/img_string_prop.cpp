/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/filename.h>           // wxFileName - encapsulates a file path
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "img_string_prop.h"

#include "art_prop_dlg.h"     // ArtBrowserDialog -- Art Property Dialog for image property
#include "project_handler.h"  // ProjectHandler class

bool ImageDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* /* property unused */)
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
        tt_cwd cwd(true);
        if (Project.HasValue(prop_art_directory))
        {
            if (auto dir = Project.ArtDirectory(); dir.dir_exists())
            {
                wxFileName::SetCwd(dir.make_wxString());
            }
        }

        wxString pattern;
        if (m_img_props.IsAnimationType())
        {
            pattern = "Animation files|*.gif;*.ani|Gif|*.gif|Ani|*.ani||";
        }
        else
        {
#ifndef __WXOSX__
            pattern = "Bitmap "
                      "files|*.png;*.bmp;*.ico;*.webp;*.xpm|PNG|*.png|Tiff|*.tif;*.tiff|WEBP|*."
                      "webp|XPM|*.xpm|Bitmaps|"
                      "*.bmp|Icon|*.ico||";
#else
            pattern =
                "Bitmap "
                "files|*.png;*.bmp;*.ico;*.webp;*.xpm|PNG|*.png|WEBP|*.webp|XPM|*.xpm|Bitmaps|"
                "*.bmp|Icon|*.ico||";
#endif
            bool remove_webp = false;
            if (Project.get_CodePreference() == GEN_LANG_CPLUSPLUS)
            {
                // WEBP was added to wxWidgets 3.3.0 -- earlier versions don't support it.
                remove_webp = (Project.get_LangVersion(GEN_LANG_CPLUSPLUS) < 30300);
            }
            else if (Project.get_CodePreference() == GEN_LANG_PYTHON)
            {
                // REVIEW: [Randalphwa - 08-31-2025] Currently, the wxPython dev has stated
                // wxWidgets 3.3.x will not be supported -- he is waiting for the stable release
                // (3.4.x). I'm guessing that the version will be wxPython 4.4.x, but until it gets
                // released, that's uncertain.
                remove_webp = Project.get_LangVersion(GEN_LANG_PYTHON) < 404000;
            }
            if (remove_webp)
            {
                pattern.Replace("|WEBP|*.webp", "", false);
                pattern.Replace(";*.webp", "", false);
            }
        }

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString,
                         pattern, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            wxFileName file(dlg.GetPath());
            file.MakeRelativeTo(Project.get_ProjectPath().make_wxString());
            auto name = file.GetFullPath();
            tt::backslashestoforward(name);
            SetValue(name);
            return true;
        }
        return false;
    }
    else if (m_img_props.type.contains("XPM") || m_img_props.type.contains("SVG"))
    {
        tt_cwd cwd(true);
        if (Project.HasValue(prop_art_directory))
        {
            if (auto dir = Project.get_ArtPath(); dir->DirExists())
            {
                dir->SetCwd();
            }
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

        wxFileDialog dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(), wxEmptyString,
                         pattern, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (dlg.ShowModal() == wxID_OK)
        {
            tt_string name = dlg.GetPath().utf8_string();
            name.make_relative(Project.get_ProjectPath());
            name.backslashestoforward();
            SetValue(name.make_wxString());
            return true;
        }
        return false;
    }

    return false;
}
