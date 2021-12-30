/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common component functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>

#include "mockup/mockup_parent.h"  // MockupParent -- Top-level MockUp Parent window

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "node_creator.h"    // NodeCreator class

#include "node_classes.h"  // Forward defintions of Node classes

class wxPropertyGridEvent;

// Common component functions

// Flags are added with no space around '|' character.
inline void AddBitFlag(ttlib::cstr& strFlags, ttlib::cview flag)
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
                          ttlib::cview def_style = tt_empty_cstr);

// Generate any non-default wxWindow settings
void GenerateWindowSettings(Node* node, ttlib::cstr& code);

// Converts bitmap property into code. Code is set to wxNullBitmap if no bitmap. Art will
// return either a bitmap or an image if scaling is requested. XPM returns wxImage and HDR
// returns GetImageFromArray() (which is a wxImage).
ttlib::cstr GenerateBitmapCode(const ttlib::cstr& description);

// Converts color text into code.
ttlib::cstr GenerateColorCode(Node* node, PropName prop_name);

ttlib::cstr GenEventCode(NodeEvent* event, const std::string& class_name);

// Will generate "wxDefaultPosition" if prop_pos is -1;-1
void GenPos(Node* node, ttlib::cstr& code);
void GenSize(Node* node, ttlib::cstr& code);

// This will output "0" if there are no styles (style, window_style, tab_position etc.)
void GenStyle(Node* node, ttlib::cstr& code);

// Returns the integer value of all style properties for the node. Includes style,
// window_style, tab_position etc.
int GetStyleInt(Node* node);

// Version of GenAdditionalCode() specifically for forms
ttlib::cstr GenFormCode(GenEnum::GenCodeType command, Node* node);

ttlib::cstr GenFormSettings(Node* node);

// Add C++ escapes around any characters the compiler wouldn't accept as a normal part of a string.
ttlib::cstr ConvertToCodeString(const ttlib::cstr& text);

// Generates " = new class(" -- with class being the derived_class (if specified) or the
// normal class name.
ttlib::cstr GenerateNewAssignment(Node* node);
