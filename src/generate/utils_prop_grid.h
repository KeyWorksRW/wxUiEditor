/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid utilities
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <string_view>

#include "pugixml.hpp"  // xml read/write/create/process

class wxPropertyGridInterface;

void AfterCreationAddItems(wxPropertyGridInterface* grid_iface, Node* node);

// This will return true if wx/propgrid/advprops.h needs to be included
bool CheckAdvancePropertyInclude(Node* node);

// Returns the XRC class used by wxPropertyGridXmlHandler for this item, e.g. "wxStringProperty"
std::string GetXrcPropertyClass(Node* node);

// Returns prop_style with the zero-value default style macro removed. The default style
// macros are defined as 0 and are not registered XRC style flags, so they must not be
// written to the XRC <style> element.
std::string GetXrcPropertyGridStyle(Node* node, std::string_view default_style);

// Appends a <property> element (with <label>, <name>, optional <tip> and nested properties)
void GenXrcPropertyGridItem(Node* node, pugi::xml_node& parent);

// Appends a <property> element for every gen_propGridItem/gen_propGridCategory child
void GenXrcPropertyGridChildren(Node* node, pugi::xml_node& parent);
