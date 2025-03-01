/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

#include <wx/settings.h>

#include "gen_enums.h"  // Enumerations for generators

class wxColour;
class wxImage;
class wxPoint;
class wxSize;
class wxStyledTextCtrl;
class Node;

tt_string ClearPropFlag(tt_string_view flag, tt_string_view currentValue);
tt_string ClearMultiplePropFlags(tt_string_view flags, tt_string_view currentValue);
tt_string SetPropFlag(tt_string_view flag, tt_string_view currentValue);

// Convert a double to a string without needing to switch locales
tt_string DoubleToStr(double val);

bool isPropFlagSet(tt_string_view flag, tt_string_view currentValue);

wxSystemColour ConvertToSystemColour(tt_string_view value);

const char* ConvertFontFamilyToString(wxFontFamily family);

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
tt_string ConvertEscapeSlashes(tt_string_view str);

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
wxPoint DlgPoint(Node* node, GenEnum::PropName prop);

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
wxSize DlgSize(Node* node, GenEnum::PropName prop);

// Given a width this will convert it using wxGetMainFrame()->getWindow()->FromDIP()
int DlgPoint(int width);

// Convert a filename to a valid variable name. This will handle filnames with leading
// numbers, utf8 characters, and other characters that are not valid in a variable name.
//
// If max_length is exceeded, the name will be have ""_name_truncated" as a suffix
std::optional<tt_string> FileNameToVarName(tt_string_view filename, size_t max_length = 256);

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
wxSize GetSizeInfo(tt_string_view size_description);

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;

bool isConvertibleMime(const tt_string& suffix);

// Checks whether a string is a valid C++ variable name.
bool isValidVarName(const std::string& str, GenLang language = GEN_LANG_CPLUSPLUS);

// This takes the class_name of the form, converts it to lowercase, and if the class name
// ends with Base, the a "_base" suffix is added.
//
// This does *not* check to see if the file already exists.
tt_string CreateBaseFilename(Node* form_node, const tt_string& class_name);

tt_string CreateDerivedFilename(Node* form_node, const tt_string& class_name);

// Typically called to convert a string into a Ruby string which prefers snake_case
tt_string ConvertToSnakeCase(tt_string_view str);

// Converts string to snake_case, then converts to upper case
tt_string ConvertToUpperSnakeCase(tt_string_view str);

// Returns false if property contains a 'n', or language is C++ and wxWidgets 3.1 is being
// used.
bool isScalingEnabled(Node* node, GenEnum::PropName prop_name, GenLang m_language = GEN_LANG_NONE);

// Convert the GEN_LANG enum to a string
std::string_view ConvertFromGenLang(GenLang language);

GenLang ConvertToGenLang(tt_string_view language);

std::string GetLanguageExtension(GenLang language);

// This will set the lexer and colors taking into account the user's preferences for dark
// mode, and specific language colors
void SetStcColors(wxStyledTextCtrl* stc, GenLang language, bool set_lexer = true, bool add_keywords = true);
