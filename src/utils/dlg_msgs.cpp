/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxMessageDialog dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "dlg_msgs.h"

//////////////// Import Project Dialogs ////////////////////////

void dlgCannotParse(const pugi::xml_parse_result& result, const std::string& filename, const std::string& caption)
{
    std::string msg("Unable to load\n    \"" + filename + "\"\n" + result.description());
    wxMessageDialog dlg(nullptr, msg, caption, wxICON_ERROR | wxOK);
    dlg.ShowModal();
}

void dlgInvalidProject(const std::string& filename, std::string_view project_type, std::string_view caption)
{
#ifdef __cpp_lib_format
    std::string msg = std::format("The file\n    \"{}\"\nis not a valid {} file.", filename, project_type);
#else
    tt_string msg;
    msg << "The file\n    \"" << filename << "\"\nis not a valid " << project_type << " file.";
#endif  // __cpp_lib_format
    wxMessageDialog dlg(nullptr, msg, wxString(caption), wxICON_ERROR | wxOK);
    dlg.ShowModal();
}

void dlgImportError(const std::exception& err, const std::string& filename, std::string_view caption)
{
    std::string msg("An internal error occurred (" + std::string(err.what()) + ") trying to import\n    \"" + filename +
                    "\"\nThis project appears to be invalid and cannot be loaded.");
    wxMessageDialog dlg(nullptr, msg, wxString(caption), wxICON_ERROR | wxOK);
    dlg.ShowModal();
}

//////////////// Code Generation Dialogs ////////////////////////

void dlgGenInternalError(const std::exception& err, const std::string& filename, const std::string& caption)
{
    std::string msg("An internal error occurred (" + std::string(err.what()) + ") generating\n    \"" + filename + "\"\n");
    wxMessageDialog dlg(nullptr, msg, caption, wxICON_ERROR | wxOK);
    dlg.ShowModal();
}
