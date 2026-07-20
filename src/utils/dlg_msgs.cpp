/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxMessageDialog dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-16-2026]
#include <format>

#include "dlg_msgs.h"

//////////////// Import Project Dialogs ////////////////////////

void dlgCannotParse(const pugi::xml_parse_result& result, const std::string& filename,
                    const std::string& caption)
{
    const std::string msg("Unable to load\n    \"" + filename + "\"\n" + result.description());
    wxMessageDialog dialog(nullptr, msg, caption, wxICON_ERROR | wxOK);
    dialog.ShowModal();
}

void dlgInvalidProject(const std::string& filename, std::string_view project_type,
                       std::string_view caption)
{
    const std::string msg =
        std::format("The file\n    \"{}\"\nis not a valid {} file.", filename, project_type);
    wxMessageDialog dialog(nullptr, msg, wxString(caption), wxICON_ERROR | wxOK);
    dialog.ShowModal();
}

void dlgImportError(const std::exception& err, const std::string& filename,
                    std::string_view caption)
{
    const std::string msg("An internal error occurred (" + std::string(err.what()) +
                          ") trying to import\n    \"" + filename +
                          "\"\nThis project appears to be invalid and cannot be loaded.");
    wxMessageDialog dialog(nullptr, msg, wxString(caption), wxICON_ERROR | wxOK);
    dialog.ShowModal();
}

//////////////// Code Generation Dialogs ////////////////////////

void dlgGenInternalError(const std::exception& err, const std::string& filename,
                         const std::string& caption)
{
    const std::string msg("An internal error occurred (" + std::string(err.what()) +
                          ") generating\n    \"" + filename + "\"\n");
    wxMessageDialog dialog(nullptr, msg, caption, wxICON_ERROR | wxOK);
    dialog.ShowModal();
}
