/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for handling wxImage files or art
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/filedlg.h>            // wxFileDialog base header
#include <wx/filename.h>           // wxFileName - encapsulates a file path
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "img_string_prop.h"

#include "art_prop_dlg.h"                // ArtPropertyDlg -- Art Property Dialog for image property
#include "project_handler.h"             // ProjectHandler class
#include "version.h"                     // Version numbers and other constants
#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::SaveCwd

[[nodiscard]] bool ImageDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid,
                                                    [[maybe_unused]] wxPGProperty* property)
{
    if (m_img_props.type.contains("Art"))
    {
        ArtPropertyDlg art_dlg(propGrid->GetPanel(), m_img_props);
        if (art_dlg.ShowModal() == wxID_OK)
        {
            SetValue(art_dlg.GetResults());
            return true;
        }
        return false;
    }
    if (m_img_props.type.contains("Embed"))
    {
        wxue::SaveCwd const save_cwd(wxue::restore_cwd);
        if (Project.HasValue(prop_art_directory))
        {
            if (auto dir = Project.ArtDirectory(); dir.dir_exists())
            {
                wxFileName::SetCwd(dir.wx());
            }
        }

        wxString pattern;
        if (m_img_props.IsAnimationType())
        {
            pattern = "Animation files|*.gif;*.ani|Gif|*.gif|Ani|*.ani||";
        }
        else
        {
#if !defined(__WXOSX__)
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
            if (Project.get_CodePreference() == GenLang::cplusplus)
            {
                // WEBP was added to wxWidgets 3.3.0 -- earlier versions don't support it.
                remove_webp =
                    (Project.get_LangVersion(GenLang::cplusplus) < CPP_WIDGETS_VERSION_3_3_0);
            }
            else if (Project.get_CodePreference() == GenLang::python)
            {
                // REVIEW: [Randalphwa - 08-31-2025] Currently, the wxPython dev has stated
                // wxWidgets 3.3.x will not be supported -- he is waiting for the stable release
                // (3.4.x). I'm guessing that the version will be wxPython 4.4.x, but until it gets
                // released, that's uncertain.
                remove_webp = Project.get_LangVersion(GenLang::python) < 404000;
            }
            if (remove_webp)
            {
                pattern.Replace("|WEBP|*.webp", "", false);
                pattern.Replace(";*.webp", "", false);
            }
        }

        wxFileDialog file_dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(),
                              wxEmptyString, pattern, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (file_dlg.ShowModal() == wxID_OK)
        {
            wxFileName file(file_dlg.GetPath());
            file.MakeRelativeTo(Project.get_ProjectPath().wx());
            wxString name = file.GetFullPath();
            wxue::back_slashesto_forward(name);
            SetValue(name);
            return true;
        }
        return false;
    }
    if (m_img_props.type.contains("XPM") || m_img_props.type.contains("SVG"))
    {
        wxue::SaveCwd const save_cwd(wxue::restore_cwd);
        if (Project.HasValue(prop_art_directory))
        {
            if (const auto* dir = Project.get_ArtPath(); dir->DirExists())
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

        wxFileDialog file_dlg(propGrid->GetPanel(), "Open Image", wxFileName::GetCwd(),
                              wxEmptyString, pattern, wxFD_OPEN | wxFD_FILE_MUST_EXIST);
        if (file_dlg.ShowModal() == wxID_OK)
        {
            wxue::string name = file_dlg.GetPath().utf8_string();
            name.make_relative(Project.get_ProjectPath());
            name.backslashestoforward();
            SetValue(name.wx());
            return true;
        }
        return false;
    }

    return false;
}
