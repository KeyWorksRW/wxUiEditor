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

// Common component functions

inline void AddBitFlag(ttlib::cstr& strFlags, ttlib::cview flag)
{
    if (strFlags.size())
        strFlags << '|';
    strFlags << flag;
}

// Generate wxSizerFlags() function using properties from supplied node
ttlib::cstr GenerateSizerFlags(Node* node);

// If internationalize property is true, returns _("str") otherwise it just returns the string in quotes
ttlib::cstr GenerateQuotedString(const ttlib::cstr& str);

// Insert a required include file into either src or hdr set (depending on "class_access" property)
void InsertGeneratorInclude(Node* node, const std::string& include, std::set<std::string>& set_src,
                            std::set<std::string>& set_hdr);

// This is *NOT* the same as get_node_name() -- this will handle wxStaticBox and
// wxCollapsiblePane parents as well as "normal" parents
ttlib::cstr GetParentName(Node* node);

// Check for pos, size, flags, window_flags, and window name, and generate code if needed
// starting with a comma, e.g. -- ", wxPoint(x, y), wxSize(x, y), styles, name);"
void GeneratePosSizeFlags(Node* node, ttlib::cstr& code, bool uses_def_validator = false,
                          ttlib::cview extra_style = tt_empty_cstr, ttlib::cview extra_def_value = tt_empty_cstr);

// Generate any non-default wxWindow settings
void GenerateWindowSettings(Node* node, ttlib::cstr& code);

// Converts bitmap property into code. Handles both HDR and Art Provider
ttlib::cstr GenerateBitmapCode(const ttlib::cstr& description);

// Converts color text into code.
ttlib::cstr GenerateColorCode(Node* node, PropName prop_name);

ttlib::cstr GenEventCode(NodeEvent* event, const std::string& class_name);

void GenPos(Node* node, ttlib::cstr& code);
void GenSize(Node* node, ttlib::cstr& code);

// This will output "0" if style, win_style, and extra_style are all empty
void GenStyle(Node* node, ttlib::cstr& code, ttlib::cview extra_style = tt_empty_cstr,
              ttlib::cview extra_def_value = tt_empty_cstr);

ttlib::cstr GenFormCode(const std::string& cmd, Node* node, const std::string& class_name);
ttlib::cstr GenFormSettings(Node* node);
