/////////////////////////////////////////////////////////////////////////////
// Purpose:   Common XRC generating functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class
#include "node_classes.h"    // Forward defintions of Node classes

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

#include "pugixml.hpp"  // xml read/write/create/process

class FontProperty;

extern const char* g_xrc_keywords;

// Write sizeritem XRC code
void GenXrcSizerItem(Node* node, pugi::xml_node& object);

// Generates style | windows_style, pos, and size
void GenXrcStylePosSize(Node* node, pugi::xml_node& object, PropName other_style = prop_unknown);

// Call this variant when you need to preprocess the style to remove unusable flags
void GenXrcPreStylePosSize(Node* node, pugi::xml_node& object, std::string_view processed_style);

// These are the settings read by wxXmlResourceHandlerImpl::SetupWindow
void GenXrcWindowSettings(Node* node, pugi::xml_node& object);

// Add comments for unsupported properties
void GenXrcComments(Node* node, pugi::xml_node& object, size_t supported_flags = xrc::all_unsupported);

// Specify the XRC param name if it doesn't match the property name
void GenXrcBitmap(Node* node, pugi::xml_node& item, size_t xrc_flags, std::string_view param_name = {});

// Generates class and name attributes for the object.
//
// If prop_id is not set to wxID_ANY, then it will be used as the name instead of
// prop_var_name
void GenXrcObjectAttributes(Node* node, pugi::xml_node& object, std::string_view xrc_class);

// If parent is a sizer, generates a sizer_item, and returns a newly created object.
//
// If parent is not a sizer, it simply returns the object.
pugi::xml_node InitializeXrcObject(Node* node, pugi::xml_node& object);

void GenXrcFont(pugi::xml_node& object, FontProperty& font_prop);

// param_name is the name of the XRC object, typically "font".
void GenXrcFont(pugi::xml_node& item, std::string_view param_name, Node* node, PropName prop);

// Can be used for either wxToolBar or wxAuiToolBar
void GenXrcToolProps(Node* node, pugi::xml_node& item, size_t xrc_flags);
