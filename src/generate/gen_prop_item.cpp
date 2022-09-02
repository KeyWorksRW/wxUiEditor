/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid/Manager Item generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_prop_item.h"

std::optional<ttlib::cstr> PropertyGridItemGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = " << node->get_parent_name();

    if (node->prop_as_string(prop_type) == "Category")
    {
        code << "->Append(new wxPropertyCategory(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string(prop_label)) << "));";
    }
    else
    {
        code << "->Append(new wx" << node->prop_as_string(prop_type) << "Property(";
        code << GenerateQuotedString(node->prop_as_string(prop_label)) << ", "
             << GenerateQuotedString(node->prop_as_string(prop_help)) << "));";
    }

    return code;
}

ttlib::cstr PropertyGridItemGenerator::GetHelpURL(Node* node)
{
    ttlib::cstr type = node->prop_as_string(prop_type);
    type.MakeLower();
    ttlib::cstr url = "wx_";

    if (type == "category")
    {
        url << "property_category.html";
    }

    else
    {
        if (!type.starts_with("string"))
            type.Replace("string", "_string");
        if (!type.starts_with("choice"))
            type.Replace("choice", "_choice");
        if (!type.starts_with("colour"))
            type.Replace("colour", "_colour");
        if (!type.starts_with("enum"))
            type.Replace("enum", "_enum");
        if (!type.starts_with("int"))
            type.Replace("int", "_int");
        if (!type.starts_with("file"))
            type.Replace("file", "_file");

        url << type << "_property.html";
    }

    return url;
}

ttlib::cstr PropertyGridItemGenerator::GetHelpText(Node* node)
{
    ttlib::cstr help_text("wx");
    if (node->prop_as_string(prop_type) == "Category")
        help_text << "PropertyCategory";
    else
        help_text << node->prop_as_string(prop_type) << "Property";
    return help_text;
}
