/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxMessageDialog dialogs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

//////////////// Import Project Dialogs ////////////////////////

void dlgCannotParse(const pugi::xml_parse_result& result, const std::string& filename, const std::string& caption);
void dlgInvalidProject(const std::string& filename, std::string_view project_type, std::string_view caption);
void dlgImportError(const std::exception& err, const std::string& filename, std::string_view caption);

//////////////// Code Generation Dialogs ////////////////////////

void dlgGenInternalError(const std::exception& err, const std::string& filename, const std::string& caption);
