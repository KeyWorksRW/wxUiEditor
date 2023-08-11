/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGridCategory (propGridItem) generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_prop_category.h"

bool PropertyGridCategoryGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().Str(" = ").ParentName();
    code.Function("Append(").AddIfCpp("new ").Add("wxPropertyCategory").AddIfRuby(".new").Str("(");
    code.QuotedString(prop_label).Comma().QuotedString(prop_label).Str(")").EndFunction();

    return true;
}

tt_string PropertyGridCategoryGenerator::GetHelpURL(Node* /* node */)
{
    tt_string url = "wx_";
    url << "property_category.html";

    return url;
}

tt_string PropertyGridCategoryGenerator::GetHelpText(Node* /* node */)
{
    tt_string help_text("wxPropertyCategory");
    return help_text;
}
