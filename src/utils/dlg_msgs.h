/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxMessageDialog dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file declares utility functions for displaying error dialogs during project
// import and code generation operations. Import functions (dlgCannotParse, dlgInvalidProject,
// dlgImportError) handle XML parsing failures, unsupported project formats, and general exceptions
// with contextual error messages including filename and project type. Code generation function
// (dlgGenInternalError) reports exceptions during code generation with file context. All functions
// wrap wxMessageDialog creation to provide consistent error presentation with appropriate icons,
// buttons, and caption text. They accept pugi::xml_parse_result, std::exception, std::string_view
// parameters enabling detailed error reporting from importers (FormBuilder, Glade, XRC, etc.) and
// generators (C++, Python, Ruby, Perl) to guide users in troubleshooting failed operations.

#pragma once

#include <pugixml.hpp>

//////////////// Import Project Dialogs ////////////////////////

void dlgCannotParse(const pugi::xml_parse_result& result, const std::string& filename,
                    const std::string& caption);
void dlgInvalidProject(const std::string& filename, std::string_view project_type,
                       std::string_view caption);
void dlgImportError(const std::exception& err, const std::string& filename,
                    std::string_view caption);

//////////////// Code Generation Dialogs ////////////////////////

void dlgGenInternalError(const std::exception& err, const std::string& filename,
                         const std::string& caption);
