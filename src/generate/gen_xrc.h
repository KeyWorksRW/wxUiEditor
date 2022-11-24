/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate XRC file
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;

namespace pugi
{
    class xml_node;
}

// Generate a string containing the XRC of the starting node and all of it's children.
//
// Use project node to generate an XRC string of the entire project.
std::string GenerateXrcStr(Node* node_start, size_t xrc_flags);

int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags);
extern const char* txt_dlg_name;  // "_wxue_temp_dlg";
