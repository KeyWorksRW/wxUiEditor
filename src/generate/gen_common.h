/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common code generation functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <optional>
#include <vector>

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "code.h"            // Code -- Helper class for generating code
#include "node_creator.h"    // NodeCreator class

#include "node_classes.h"  // Forward defintions of Node classes

#include "gen_xrc_utils.h"  // Common XRC generating functions

class wxPropertyGridEvent;

class FontProperty;

// Common component functions

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
tt_string GenerateQuotedString(const tt_string& str);

tt_string GenerateQuotedString(Node* node, GenEnum::PropName prop_name);

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
tt_string ConvertToCodeString(const tt_string& text);

// Insert a required include file into either src or hdr set (depending on prop_class_access)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as getNodeName() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
tt_string GetParentName(Node* node, GenLang language);

// Used for controls that need to call SetBitmap(bitmap). Returns true if wxVector generated.
//
// Set is_single to true for a non-button control.
bool GenBtnBimapCode(Node* node, tt_string& code, bool is_single = false);

// Converts bitmap property into code. Code is set to wxNullBitmap if no bitmap. Art will
// return either a bitmap or an image if scaling is requested. XPM returns wxImage and HDR
// returns wxueImage() (which is a wxImage). pDpiWindow is the name of the window
// to use for wxBitmapBundle::GetBitmapFrom()
tt_string GenerateBitmapCode(const tt_string& description);

// Generates the code necessary to create a wxBitmapBundle used to pass as an argument to a
// function, or the start of a code block containing: { wxVector<wxBitmap> bitmaps;
//
// Returns true if the returned string contains a wxVector. For 3.1 code, this will start
// with "#if wxCHECK_VERSION(3, 1, 6)" and caller must supply #else and #endif.
bool GenerateBundleCode(const tt_string& description, tt_string& code);

// Returns the integer value of all style properties for the node. Includes style,
// window_style, tab_position etc.
//
// If style is a friendly name, add the prefix parameter to prefix lookups.
int GetStyleInt(Node* node, const char* prefix = nullptr);

// Returns the integer value of the type_bitlist property for the node.
//
// Note: requires each bitlist option to have been added to node_constants.cpp
int GetBitlistInt(Node* node, GenEnum::PropName prop_name);

// This generates code for the header file for Get() and Set() functions using function names
// specified by the user in the project file.
std::optional<tt_string> GenGetSetCode(Node* node);

void GenValidatorSettings(Code& code);

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate
// everything needed to set the window's icon.
tt_string GenerateIconCode(const tt_string& description);

// Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
// Generates wxNullColour if the property is empty.
void ColourCode(Code& code, GenEnum::PropName prop_name);

struct GenResults;
// Common function to generate all files for a specific language
bool GenerateLanguageFiles(GenResults& results, std::vector<tt_string>* pClassList, GenLang language);

// Called by GenerateLanguageFiles and in OnGenSingle...() to generate a single form
bool GenerateLanguageForm(Node* form, GenResults& results, std::vector<tt_string>* pClassList, GenLang language);

void OnGenerateSingleLanguage(GenLang language);
void OnGenerateLanguage(GenLang language);

/////////////////////////////////////// Code-enabled Functions ///////////////////////////////////////

// Generate settings common to all forms
void GenFormSettings(Code& code);

// This is called to add a tool to wxToolBar, wxAuiToolBar or wxRibbonToolBar
void GenToolCode(Code& code);

// Returns true if a bitmaps vector was created (C++ or Python)
//
// C++ Caller should add the function that uses the bitmaps, add the closing brace, and if
// prop_wxWidgets_version == 3.1, follow this with a #else and the alternate code.
bool BitmapList(Code& code, const GenEnum::PropName prop);

/////////////////////////////////////// wxPerl Functions ///////////////////////////////////////

// Deterimes where the perl code will be written to, and returns an absolute path to that
// location.
tt_string MakePerlPath(Node* node);

// Generates code to load a bitmap from Art, SVG, or up to three bitmap files.
bool PerlBundleCode(Code& code, GenEnum::PropName prop);

/////////////////////////////////////// wxPython Functions ///////////////////////////////////////

// Returns true if a list was created. List name will be called "bitmaps".
bool PythonBitmapList(Code& code, GenEnum::PropName prop);

// Generates code to load a bitmap from Art, SVG, or up to two bitmap files.
bool PythonBundleCode(Code& code, GenEnum::PropName prop);

// Deterimes where the python code will be written to, and returns an absolute path to that
// location.
tt_string MakePythonPath(Node* node);

// Python version of GenBtnBimapCode()
void PythonBtnBimapCode(Code& code, bool is_single = false);

/////////////////////////////////////// wxRuby Functions ///////////////////////////////////////

// Deterimes where the ruby code will be written to, and returns an absolute path to that
// location.
tt_string MakeRubyPath(Node* node);

// Generates code to load a bitmap from Art, SVG, or up to three bitmap files.
bool RubyBundleCode(Code& code, GenEnum::PropName prop);

/////////////////////////////////////// wxRust Functions ///////////////////////////////////////

// Deterimes where the perl code will be written to, and returns an absolute path to that
// location.
tt_string MakeRustPath(Node* node);
