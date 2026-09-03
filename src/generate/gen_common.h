/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common code generation functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <vector>

#include "base_generator.h"              // BaseGenerator -- Base Generator class
#include "code.h"                        // Code -- Helper class for generating code
#include "wxue_namespace/wxue_string.h"  // wxue::string

class Node;

class wxPropertyGridEvent;

class FontProperty;

// Common component functions

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is set. The _() wrapper is omitted if the string contains no alphabetic
// characters (e.g. "123", "%%", "1,000").
//
// Will return "wxEmptyString" if prop_name is empty.
wxue::string GenerateQuotedString(const wxue::string& str);

// Insert a required include file into either src or hdr set (depending on prop_class_access)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as get_NodeName() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
wxue::string get_ParentName(Node* node, GenLang language);

// Used for controls that need to call SetBitmap(bitmap). Returns true if wxVector generated.
//
// Set is_single to true for a non-button control.
bool GenBtnBitmapCode(Node* node, wxue::string& code, bool is_single = false);

// Converts bitmap property into code. Code is set to wxNullBitmap if no bitmap. Art will
// return either a bitmap or an image if scaling is requested. XPM returns wxImage and HDR
// returns wxueImage() (which is a wxImage). pDpiWindow is the name of the window
// to use for wxBitmapBundle::GetBitmapFrom()
wxue::string GenerateBitmapCode(const wxue::string& description);

// Generates the code necessary to create a wxBitmapBundle used to pass as an argument to a
// function, or the start of a code block containing: { wxVector<wxBitmap> bitmaps;
//
// Returns true if the returned string contains a wxVector.
bool GenerateBundleCode(const wxue::string& description, wxue::string& code);

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
std::optional<wxue::string> GenGetSetCode(Node* node);

void GenValidatorSettings(Code& code);

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate
// everything needed to set the window's icon.
wxue::string GenerateIconCode(const wxue::string& description);

class GenResults;

void OnGenerateSingleLanguage(GenLang language);

/////////////////////////////////////// Code-enabled Functions /////////////////////////////////

// Generate settings common to all forms
void GenFormSettings(Code& code);

// This is called to add a tool to wxToolBar, wxAuiToolBar or wxRibbonToolBar
void GenToolCode(Code& code);

// Returns true if a bitmaps vector was created (C++ or Python)
//
// C++ Caller should add the function that uses the bitmaps, add the closing brace
bool BitmapList(Code& code, GenEnum::PropName prop);

/////////////////////////////////////// wxPython Functions ///////////////////////////////////////

// Returns true if a list was created. List name will be called "bitmaps".
bool PythonBitmapList(Code& code, GenEnum::PropName prop);

// Deterimes where the python code will be written to, and returns an absolute path to that
// location.
wxue::string MakePythonPath(Node* node);

// Python version of GenBtnBitmapCode()
void PythonBtnBitmapCode(Code& code, bool is_single = false);

/////////////////////////////////////// wxRuby Functions ///////////////////////////////////////

// Deterimes where the ruby code will be written to, and returns an absolute path to that
// location.
wxue::string MakeRubyPath(Node* node);
