/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-16-2026]
#pragma once

#include <optional>

#include <wx/settings.h>

#include "gen_enums.h"                   // Enumerations for generators
#include "wxue_namespace/wxue_string.h"  // wxue::string -- std::string with utility methods

// Default project file extension (also accepts ".wxue" for legacy reasons)
inline constexpr std::string_view PROJECT_FILE_EXTENSION = ".wxui";
inline constexpr std::string_view PROJECT_LEGACY_FILE_EXTENSION = ".wxue";

class wxColour;
class wxImage;
class wxPoint;
class wxSize;
class wxStyledTextCtrl;
class Node;
class wxInputStream;
class wxOutputStream;

// Structure to hold adjacent Node* parameters for form and child. This allows C++20 designated
// initializers to be used.
struct NodesFormChild
{
    Node* form;
    Node* child;
};

// Newer functions should be placed in the utils namespace
namespace utils
{
    // Look for search string in line, and if found, replace with replace_with string. If all
    // is true, replace all instances, otherwise only the first instance is replaced.
    void replace_in_line(std::string& line, std::string_view search, std::string_view replace_with,
                         bool replace_all);
}  // namespace utils

wxue::string ClearPropFlag(wxue::string_view flag, wxue::string_view currentValue);
wxue::string ClearMultiplePropFlags(wxue::string_view flags, wxue::string_view currentValue);
wxue::string SetPropFlag(wxue::string_view flag, wxue::string_view currentValue);

// Convert a double to a string without needing to switch locales
wxue::string DoubleToStr(double val);

[[nodiscard]] bool isPropFlagSet(wxue::string_view flag, wxue::string_view currentValue);

wxSystemColour ConvertToSystemColour(wxue::string_view value);

const char* ConvertFontFamilyToString(wxFontFamily family);

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
wxue::string ConvertEscapeSlashes(wxue::string_view str);

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
wxPoint DlgPoint(Node* node, GenEnum::PropName prop);

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
wxSize DlgSize(Node* node, GenEnum::PropName prop);

// Given a width this will convert it using wxGetMainFrame()->getWindow()->FromDIP()
int DlgPoint(int width);

// Convert a filename to a valid variable name. This will handle filenames with leading
// numbers, utf8 characters, and other characters that are not valid in a variable name.
//
// If max_length is exceeded, the name will be have ""_name_truncated" as a suffix
std::optional<wxue::string> FileNameToVarName(wxue::string_view filename, size_t max_length = 256);

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
wxSize GetSizeInfo(wxue::string_view size_description);

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;  // NOLINT () // cppcheck-suppress

[[nodiscard]] bool isConvertibleMime(const wxue::string& suffix);

// Checks whether a string is a valid C++ variable name.
[[nodiscard]] bool isValidVarName(const std::string& str, GenLang language = GenLang::cplusplus);

// This takes the class_name of the form, converts it to lowercase, and if the class name
// ends with Base, the a "_base" suffix is added.
//
// This does *not* check to see if the file already exists.
wxue::string CreateBaseFilename(Node* form_node, const wxue::string& class_name);

wxue::string CreateDerivedFilename(Node* form_node, const wxue::string& class_name);

// Typically called to convert a string into a Ruby string which prefers snake_case
std::string ConvertToSnakeCase(std::string_view str);

// Converts string to snake_case, then converts to upper case
wxue::string ConvertToUpperSnakeCase(wxue::string_view str);

// Returns false if property contains a 'n', or language is C++ and wxWidgets 3.1 is being
// used.
[[nodiscard]] bool isScalingEnabled(Node* node, GenEnum::PropName prop_name,
                                    GenLang m_language = GenLang::none);

// Convert the GEN_LANG enum to a string
std::string_view GenLangToString(GenLang language);

GenLang ConvertToGenLang(wxue::string_view language);

std::string GetLanguageExtension(GenLang language);

enum class ClassOverrideType : std::uint8_t
{
    None = 0,
    Subclass,  // User specified a subclass
    Generic,   // Use the wxGeneric version of the class
};
ClassOverrideType GetClassOverrideType(Node* node);

// This will set the lexer and colors taking into account the user's preferences for dark
// mode, and specific language colors
void SetStcColors(wxStyledTextCtrl* stc, GenLang language, bool set_lexer = true,
                  bool add_keywords = true);

// Call this after creating a wxRibbonBar tool in order to ensure that it has a unique ID/
void SetUniqueRibbonToolID(Node* node);

// Normally, wxMemoryInputStream inputStream, wxZlibOutputStream outputStream
[[nodiscard]] bool CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream,
                                  size_t compressed_size);

// Show the Open/Import Project file dialog and return the selected filename.
// Returns an empty string if the user cancels.
wxString ShowOpenProjectDialog(wxWindow* parent);
