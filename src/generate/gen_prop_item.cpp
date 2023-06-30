/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGridItem (propGridItem) generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_prop_item.h"

bool PropertyGridItemGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").ValidParentName();
    // .Function("Append(new wx").PropAs(prop_type).Str("Property(");

    if (code.view(prop_type) == "Category")
    {
        // Break out the final '(' so that lookup for wxPropertyCategory will find the propgrid
        // library
        code.Function("Append(").Str(code.is_cpp() ? "new " : "").Add("wxPropertyCategory").Str("(");
        code.QuotedString(prop_label).Comma().QuotedString(prop_label).Str(")").EndFunction();
    }
    else
    {
        tt_string name("wx");
        name << code.node()->value(prop_type) << "Property";
        code.Function("Append(").AddIfCpp("new ").Add(name).Str("(");
        code.QuotedString(prop_label).Comma().QuotedString(prop_help).Str(")").EndFunction();
    }

    return true;
}

tt_string PropertyGridItemGenerator::GetHelpURL(Node* node)
{
    tt_string type = node->as_string(prop_type);
    type.MakeLower();
    tt_string url = "wx_";

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

tt_string PropertyGridItemGenerator::GetHelpText(Node* node)
{
    tt_string help_text("wx");
    if (node->as_string(prop_type) == "Category")
        help_text << "PropertyCategory";
    else
        help_text << node->as_string(prop_type) << "Property";
    return help_text;
}
