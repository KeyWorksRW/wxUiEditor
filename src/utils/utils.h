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

auto ClearPropFlag(tt_string_view flag, tt_string_view currentValue) -> tt_string;
auto ClearMultiplePropFlags(tt_string_view flags, tt_string_view currentValue) -> tt_string;
auto SetPropFlag(tt_string_view flag, tt_string_view currentValue) -> tt_string;

// Convert a double to a string without needing to switch locales
auto DoubleToStr(double val) -> tt_string;

auto isPropFlagSet(tt_string_view flag, tt_string_view currentValue) -> bool;

auto ConvertToSystemColour(tt_string_view value) -> wxSystemColour;

auto ConvertFontFamilyToString(wxFontFamily family) -> const char*;

// Replace escape slashes with the actual character. Affects \\, \\n, \\r, and \\t
auto ConvertEscapeSlashes(tt_string_view str) -> tt_string;

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
auto DlgPoint(Node* node, GenEnum::PropName prop) -> wxPoint;

// This will *always* call wxGetMainFrame()->getWindow()->FromDIP()
auto DlgSize(Node* node, GenEnum::PropName prop) -> wxSize;

// Given a width this will convert it using wxGetMainFrame()->getWindow()->FromDIP()
auto DlgPoint(int width) -> int;

// Convert a filename to a valid variable name. This will handle filnames with leading
// numbers, utf8 characters, and other characters that are not valid in a variable name.
//
// If max_length is exceeded, the name will be have ""_name_truncated" as a suffix
auto FileNameToVarName(tt_string_view filename, size_t max_length = 256)
    -> std::optional<tt_string>;

// Convert the parts[IndexSize] or equivalent string into wxSize dimensions
auto GetSizeInfo(tt_string_view size_description) -> wxSize;

// Friendly name/wxSTC_WRAP_ constant
extern std::map<std::string, const char*> g_stc_wrap_mode;  // NOLINT () // cppcheck-suppress

auto isConvertibleMime(const tt_string& suffix) -> bool;

// Checks whether a string is a valid C++ variable name.
auto isValidVarName(const std::string& str, GenLang language = GEN_LANG_CPLUSPLUS) -> bool;

// This takes the class_name of the form, converts it to lowercase, and if the class name
// ends with Base, the a "_base" suffix is added.
//
// This does *not* check to see if the file already exists.
auto CreateBaseFilename(Node* form_node, const tt_string& class_name) -> tt_string;

auto CreateDerivedFilename(Node* form_node, const tt_string& class_name) -> tt_string;

// Typically called to convert a string into a Ruby string which prefers snake_case
auto ConvertToSnakeCase(std::string_view str) -> std::string;

// Converts string to snake_case, then converts to upper case
auto ConvertToUpperSnakeCase(tt_string_view str) -> tt_string;

// Returns false if property contains a 'n', or language is C++ and wxWidgets 3.1 is being
// used.
auto isScalingEnabled(Node* node, GenEnum::PropName prop_name, GenLang m_language = GEN_LANG_NONE)
    -> bool;

// Convert the GEN_LANG enum to a string
auto GenLangToString(GenLang language) -> std::string_view;

auto ConvertToGenLang(tt_string_view language) -> GenLang;

auto GetLanguageExtension(GenLang language) -> std::string;

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
auto CopyStreamData(wxInputStream* inputStream, wxOutputStream* outputStream,
                    size_t compressed_size) -> bool;
