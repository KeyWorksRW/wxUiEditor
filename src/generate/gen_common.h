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
#include "node_creator.h"    // NodeCreator class

#include "node_classes.h"  // Forward defintions of Node classes

class wxPropertyGridEvent;

// Common component functions

// Flags are added with no space around '|' character.
inline void AddBitFlag(ttlib::cstr& strFlags, ttlib::sview flag)
{
    if (strFlags.size())
        strFlags << '|';
    strFlags << flag;
}

// Generate wxSizerFlags() function based on prop_proportion, prop_alignment, prop_flags,
// prop_borders and prop_border_size
ttlib::cstr GenerateSizerFlags(Node* node);

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
ttlib::cstr GenerateQuotedString(const ttlib::cstr& str);

// Places the string in wxString::FromUTF8(), adds C++ escapes around any characters the
// compiler wouldn't accept as a normal part of a string, and wraps it all in _() if
// prop_internationalize is true.
//
// Will return "wxEmptyString" if prop_name is empty.
ttlib::cstr GenerateQuotedString(Node* node, GenEnum::PropName prop_name);

// Creates a string using either wxSystemSettings::GetColour(name) or wxColour(r, g, b).
// Generates wxNullColour if the property is empty.
ttlib::cstr GenerateColourCode(Node* node, GenEnum::PropName prop_name);

// Insert a required include file into either src or hdr set (depending on prop_class_access)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as get_node_name() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
ttlib::cstr GetParentName(Node* node);

// Check for pos, size, flags, window_flags, and window name, and generate code if needed
// starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
//
// If the only style specified is def_style, then it will not be added.
void GeneratePosSizeFlags(Node* node, ttlib::cstr& code, bool uses_def_validator = false,
                          ttlib::sview def_style = tt_empty_cstr);

// Generate any non-default wxWindow settings
void GenerateWindowSettings(Node* node, ttlib::cstr& code);

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

ttlib::cstr GenEventCode(NodeEvent* event, const std::string& class_name);

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

// Version of GenAdditionalCode() specifically for forms
ttlib::cstr GenFormCode(GenEnum::GenCodeType command, Node* node);

ttlib::cstr GenFormSettings(Node* node);

// Generates font, foreground, and background settings if any of them have been specified.
ttlib::cstr GenFontColourSettings(Node* node);

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
ttlib::cstr ConvertToCodeString(const ttlib::cstr& text);

// Generates " = new class(" -- with class being the derived_class (if specified) or the
// normal class name. If use_generic specified, Generic will be inserted into the class name.
ttlib::cstr GenerateNewAssignment(Node* node, bool use_generic = false);

// This generates code for the header file for Get() and Set() functions using function names
// specified by the user in the project file.
std::optional<ttlib::cstr> GenGetSetCode(Node* node);

std::optional<ttlib::cstr> GenValidatorSettings(Node* node);

// Write sizeritem XRC code
void GenXrcSizerItem(Node* node, BaseCodeGenerator* code_gen);

void GenXrcSizerItem(Node*, pugi::xml_node& object);
