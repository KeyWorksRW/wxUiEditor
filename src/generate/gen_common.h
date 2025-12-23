/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common code generation functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements utility functions shared across all code generators for handling
// common wxWidgets patterns. Key responsibilities: string escaping and internationalization
// (GenerateQuotedString, ConvertToCodeString), bitmap/bundle code generation (GenerateBitmapCode,
// GenerateBundleCode, GenBtnBimapCode), style bitflag conversion (GetStyleInt, GetBitlistInt), and
// language-specific file generation orchestration (GenerateLanguageFiles, GenerateLanguageForm).
// Helper functions generate code for colors (ColourCode), fonts, icons (GenerateIconCode), parent
// name resolution (get_ParentName), validators (GenValidatorSettings), and Get/Set accessor methods
// (GenGetSetCode). Language-specific sections provide path resolution (MakePythonPath,
// MakePerlPath, MakeRubyPath), bitmap list generation (PythonBitmapList, PerlBitmapList), and event
// gathering (GatherPerlNodeEvents). These functions are called by both BaseCodeGenerator derived
// classes and individual BaseGenerator widget generators to ensure consistent code generation
// across languages.

#pragma once

#include <optional>
#include <vector>

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "code.h"            // Code -- Helper class for generating code

class Node;

class wxPropertyGridEvent;

class FontProperty;

// Common component functions

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
auto GenerateQuotedString(const tt_string& str) -> tt_string;

auto GenerateQuotedString(Node* node, GenEnum::PropName prop_name) -> tt_string;

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
auto ConvertToCodeString(const tt_string& text) -> tt_string;

// Insert a required include file into either src or hdr set (depending on prop_class_access)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as get_NodeName() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
auto get_ParentName(Node* node, GenLang language) -> tt_string;

// Used for controls that need to call SetBitmap(bitmap). Returns true if wxVector generated.
//
// Set is_single to true for a non-button control.
auto GenBtnBimapCode(Node* node, tt_string& code, bool is_single = false) -> bool;

// Converts bitmap property into code. Code is set to wxNullBitmap if no bitmap. Art will
// return either a bitmap or an image if scaling is requested. XPM returns wxImage and HDR
// returns wxueImage() (which is a wxImage). pDpiWindow is the name of the window
// to use for wxBitmapBundle::GetBitmapFrom()
auto GenerateBitmapCode(const tt_string& description) -> tt_string;

// Generates the code necessary to create a wxBitmapBundle used to pass as an argument to a
// function, or the start of a code block containing: { wxVector<wxBitmap> bitmaps;
//
// Returns true if the returned string contains a wxVector. For 3.1 code, this will start
// with "#if wxCHECK_VERSION(3, 1, 6)" and caller must supply #else and #endif.
auto GenerateBundleCode(const tt_string& description, tt_string& code) -> bool;

// Returns the integer value of all style properties for the node. Includes style,
// window_style, tab_position etc.
//
// If style is a friendly name, add the prefix parameter to prefix lookups.
auto GetStyleInt(Node* node, const char* prefix = nullptr) -> int;

// Returns the integer value of the type_bitlist property for the node.
//
// Note: requires each bitlist option to have been added to node_constants.cpp
auto GetBitlistInt(Node* node, GenEnum::PropName prop_name) -> int;

// This generates code for the header file for Get() and Set() functions using function names
// specified by the user in the project file.
auto GenGetSetCode(Node* node) -> std::optional<tt_string>;

void GenValidatorSettings(Code& code);

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate
// everything needed to set the window's icon.
auto GenerateIconCode(const tt_string& description) -> tt_string;

// Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
// Generates wxNullColour if the property is empty.
void ColourCode(Code& code, GenEnum::PropName prop_name);

class GenResults;

// DEPRECATED: Use GenResults::Generate() instead with SetNodes(), SetLanguages(), SetMode().
// This function is kept for backward compatibility and will be removed in a future version.
// Common function to generate all files for a specific language
[[deprecated("Use GenResults::Generate() with SetNodes(), SetLanguages(), SetMode()")]]
auto GenerateLanguageFiles(GenResults& results, std::vector<std::string>* pClassList,
                           GenLang language) -> bool;

// DEPRECATED: Use GenResults::Generate() with SetNodes(form), SetLanguages(language).
// This function is kept for backward compatibility and will be removed in a future version.
// Called by GenerateLanguageFiles and in OnGenSingle...() to generate a single form
[[deprecated("Use GenResults::Generate() with SetNodes(form), SetLanguages()")]]
auto GenerateLanguageForm(Node* form, GenResults& results, std::vector<std::string>* pClassList,
                          GenLang language) -> bool;

void OnGenerateSingleLanguage(GenLang language);
void OnGenerateLanguage(GenLang language);

/////////////////////////////////////// Code-enabled Functions /////////////////////////////////

// Generate settings common to all forms
void GenFormSettings(Code& code);

// This is called to add a tool to wxToolBar, wxAuiToolBar or wxRibbonToolBar
void GenToolCode(Code& code);

// Returns true if a bitmaps vector was created (C++ or Python)
//
// C++ Caller should add the function that uses the bitmaps, add the closing brace, and if
// prop_wxWidgets_version == 3.1, follow this with a #else and the alternate code.
auto BitmapList(Code& code, GenEnum::PropName prop) -> bool;

/////////////////////////////////////// wxPerl Functions ///////////////////////////////////////

// Returns true if a list was created. List name will be called "bitmaps".
auto PerlBitmapList(Code& code, GenEnum::PropName prop) -> bool;

// Deterimes where the perl code will be written to, and returns an absolute path to that
// location.
auto MakePerlPath(Node* node) -> tt_string;

// If returned string is non-empty, it will contain
// use "Wx::Event qw(events);"
auto GatherPerlNodeEvents(Node* node) -> tt_string;

/////////////////////////////////////// wxPython Functions ///////////////////////////////////////

// Returns true if a list was created. List name will be called "bitmaps".
auto PythonBitmapList(Code& code, GenEnum::PropName prop) -> bool;

// Generates code to load a bitmap from Art, SVG, or up to two bitmap files.
auto PythonBundleCode(Code& code, GenEnum::PropName prop) -> bool;

// Deterimes where the python code will be written to, and returns an absolute path to that
// location.
auto MakePythonPath(Node* node) -> tt_string;

// Python version of GenBtnBimapCode()
void PythonBtnBimapCode(Code& code, bool is_single = false);

/////////////////////////////////////// wxRuby Functions ///////////////////////////////////////

// Deterimes where the ruby code will be written to, and returns an absolute path to that
// location.
auto MakeRubyPath(Node* node) -> tt_string;
