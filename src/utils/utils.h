/////////////////////////////////////////////////////////////////////////////
// Purpose:   Miscellaneous utility functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file provides utility functions supporting property manipulation, code
// generation, and UI operations throughout wxUiEditor. Key categories: property flag management
// (SetPropFlag, ClearPropFlag, isPropFlagSet for bitlist properties), name conversions
// (FileNameToVarName sanitizing filenames to valid C++/ Python/Ruby identifiers, ConvertToSnakeCase
// for Ruby naming, ConvertToUpperSnakeCase for constants), validation (isValidVarName checking
// language-specific identifier rules), DPI scaling (DlgPoint, DlgSize, isScalingEnabled handling
// FromDIP conversions), language mapping (GenLangToString, ConvertToGenLang, GetLanguageExtension
// for GEN_LANG_* enums), color/font conversions (ConvertToSystemColour, ConvertFontFamilyToString),
// filename generation (CreateBaseFilename, CreateDerivedFilename for base/derived class pairs), and
// specialized helpers (SetStcColors for syntax highlighting, SetUniqueRibbonToolID for ID
// generation, CopyStreamData for zlib compression). Functions follow pattern of standalone
// utilities or namespace-grouped (utils::) for newer additions.

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
    auto replace_in_line(std::string& line, std::string_view search, std::string_view replace_with,
                         bool all) -> void;
}  // namespace utils

auto ClearPropFlag(wxue::string_view flag, wxue::string_view currentValue) -> wxue::string;
auto ClearMultiplePropFlags(wxue::string_view flags, wxue::string_view currentValue)
    -> wxue::string;
auto SetPropFlag(wxue::string_view flag, wxue::string_view currentValue) -> wxue::string;

// Convert a double to a string without needing to switch locales
auto DoubleToStr(double val) -> wxue::string;

[[nodiscard]] auto isPropFlagSet(wxue::string_view flag, wxue::string_view currentValue) -> bool;

auto ConvertToSystemColour(wxue::string_view value) -> wxSystemColour;

auto ConvertFontFamilyToString(wxFontFamily family) -> const char*;

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
auto ConvertEscapeSlashes(wxue::string_view str) -> wxue::string;

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
auto DlgPoint(Node* node, GenEnum::PropName prop) -> wxPoint;

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
auto DlgSize(Node* node, GenEnum::PropName prop) -> wxSize;

// Given a width this will convert it using wxGetMainFrame()->getWindow()->FromDIP()
auto DlgPoint(int width) -> int;

// Convert a filename to a valid variable name. This will handle filenames with leading
// numbers, utf8 characters, and other characters that are not valid in a variable name.
//
// If max_length is exceeded, the name will be have ""_name_truncated" as a suffix
auto FileNameToVarName(wxue::string_view filename, size_t max_length = 256)
    -> std::optional<wxue::string>;

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
auto GetSizeInfo(wxue::string_view size_description) -> wxSize;

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;  // NOLINT () // cppcheck-suppress

[[nodiscard]] auto isConvertibleMime(const wxue::string& suffix) -> bool;

// Checks whether a string is a valid C++ variable name.
[[nodiscard]] auto isValidVarName(const std::string& str, GenLang language = GEN_LANG_CPLUSPLUS)
    -> bool;

// This takes the class_name of the form, converts it to lowercase, and if the class name
// ends with Base, the a "_base" suffix is added.
//
// This does *not* check to see if the file already exists.
auto CreateBaseFilename(Node* form_node, const wxue::string& class_name) -> wxue::string;

auto CreateDerivedFilename(Node* form_node, const wxue::string& class_name) -> wxue::string;

// Typically called to convert a string into a Ruby string which prefers snake_case
auto ConvertToSnakeCase(std::string_view str) -> std::string;

// Converts string to snake_case, then converts to upper case
auto ConvertToUpperSnakeCase(wxue::string_view str) -> wxue::string;

// Returns false if property contains a 'n', or language is C++ and wxWidgets 3.1 is being
// used.
[[nodiscard]] auto isScalingEnabled(Node* node, GenEnum::PropName prop_name,
                                    GenLang m_language = GEN_LANG_NONE) -> bool;

// Convert the GEN_LANG enum to a string
auto GenLangToString(GenLang language) -> std::string_view;

auto ConvertToGenLang(wxue::string_view language) -> GenLang;

auto GetLanguageExtension(GenLang language) -> std::string;

enum class ClassOverrideType : std::uint8_t
{
    None = 0,
    Subclass,  // User specified a subclass
    Generic,   // Use the wxGeneric version of the class
};
auto GetClassOverrideType(Node* node) -> ClassOverrideType;

// This will set the lexer and colors taking into account the user's preferences for dark
// mode, and specific language colors
auto SetStcColors(wxStyledTextCtrl* stc, GenLang language, bool set_lexer = true,
                  bool add_keywords = true) -> void;

// Call this after creating a wxRibbonBar tool in order to ensure that it has a unique ID/
void SetUniqueRibbonToolID(Node* node);

// Normally, wxMemoryInputStream inputStream, wxZlibOutputStream outputStream
[[nodiscard]] auto CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream,
                                  size_t compressed_size) -> bool;

// Show the Open/Import Project file dialog and return the selected filename.
// Returns an empty string if the user cancels.
auto ShowOpenProjectDialog(wxWindow* parent) -> wxString;
