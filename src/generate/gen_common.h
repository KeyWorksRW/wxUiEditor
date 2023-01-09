/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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
ttlib::cstr GenerateQuotedString(const ttlib::cstr& str);

ttlib::cstr GenerateQuotedString(Node* node, GenEnum::PropName prop_name);

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
ttlib::cstr ConvertToCodeString(const ttlib::cstr& text);

// Insert a required include file into either src or hdr set (depending on prop_class_access)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as get_node_name() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
ttlib::cstr GetParentName(Node* node);

// Used for controls that need to call SetBitmap(bitmap). Returns true if wxVector generated.
//
// Set is_single to true for a non-button control.
bool GenBtnBimapCode(Node* node, ttlib::cstr& code, bool is_single = false);

// Converts bitmap property into code. Code is set to wxNullBitmap if no bitmap. Art will
// return either a bitmap or an image if scaling is requested. XPM returns wxImage and HDR
// returns wxueImage() (which is a wxImage). pDpiWindow is the name of the window
// to use for wxBitmapBundle::GetBitmapFrom()
ttlib::cstr GenerateBitmapCode(const ttlib::cstr& description);

// If a wxVector is required to create the wxBitmapBundle, this will generate the opening
// brace and the vector code and returns true with code filled in.
//
// Call this before calling GenerateBundleCode()
bool GenerateVectorCode(const ttlib::cstr& description, ttlib::cstr& code);

// Generates the code necessary to create a wxBitmapBundle used to pass as an argument to a
// function, or the start of a code block containing: { wxVector<wxBitmap> bitmaps;
//
// Returns true if the returned string starts with a '{' character and a wxVector was
// generated.
bool GenerateBundleCode(const ttlib::cstr& description, ttlib::cstr& code);

// Will generate "wxDefaultPosition" if prop_pos is -1;-1
void GenPos(Node* node, ttlib::cstr& code);
void GenSize(Node* node, ttlib::cstr& code);

// This will output "0" if there are no styles (style, window_style, tab_position etc.)
//
// If style is a friendly name, add the prefix parameter to prefix lookups.
void GenStyle(Node* node, ttlib::cstr& code, const char* prefix = nullptr);

// Returns the integer value of all style properties for the node. Includes style,
// window_style, tab_position etc.
//
// If style is a friendly name, add the prefix parameter to prefix lookups.
int GetStyleInt(Node* node, const char* prefix = nullptr);

// This generates code for the header file for Get() and Set() functions using function names
// specified by the user in the project file.
std::optional<ttlib::cstr> GenGetSetCode(Node* node);

std::optional<ttlib::cstr> GenValidatorSettings(Node* node);

// Generates code for any class inheriting from wxTopLevelWindow -- this will generate
// everything needed to set the window's icon.
ttlib::cstr GenerateIconCode(const ttlib::cstr& description);

// This is called to add a tool to either wxToolBar or wxAuiToolBar
ttlib::cstr GenToolCode(Node* node, ttlib::sview BitmapCode = "");

// Generates wxSize(x,y) or ConvertDialogToPixels(wxSize(x,y))
//
// wxSize will be converted to dialog units if the size contains a 'd' character.
ttlib::cstr GenerateWxSize(Node* node, PropName prop);

/////////////////////////////////////// Code-enabled Functions ///////////////////////////////////////

// Generate settings common to all forms
void GenFormSettings(Code& code);

// This is called to add a tool to either wxToolBar or wxAuiToolBar
void GenToolCode(Code& code, const bool is_bitmaps_list);

// Returns true if a bitmaps vector was created (C++ or Python)
//
// C++ Caller should add the function that uses the bitmaps, add the closing brace, and if
// prop_wxWidgets_version == 3.1, follow this with a #else and the alternate code.
bool BitmapList(Code& code, const GenEnum::PropName prop);

/////////////////////////////////////// wxPython Functions ///////////////////////////////////////

// Returns true if a list was created. List name will be called "bitmaps".
bool PythonBitmapList(Code& code, GenEnum::PropName prop);

// Generates code to load a bitmap from Art, SVG, or up to two bitmap files.
bool PythonBundleCode(Code& code, GenEnum::PropName prop);

// Deterimes where the python code will be written to, and returns an absolute path to that
// location which can be used as the parameter for make_relative().
ttlib::cstr MakePythonPath(Node* node);

// Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
// Generates wxNullColour if the property is empty.
void ColourCode(Code& code, GenEnum::PropName prop_name);

// Python version of GenBtnBimapCode()
void PythonBtnBimapCode(Code& code, bool is_single = false);
