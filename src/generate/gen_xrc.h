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

// If out_file contains a file, it will override project xrc_file and combine_xrc settings.
//
// If NeedsGenerateCheck is true, this will not write any files, but will return true if at
// least one file needs to be generated.
//
// If pClassList is non-null, it will contain the base class name of every form that needs
// updating.
bool GenerateXrcFiles(ttlib::cstr out_file = {}, bool NeedsGenerateCheck = false);

extern const char* txt_dlg_name;  // "_wxue_temp_dlg";
