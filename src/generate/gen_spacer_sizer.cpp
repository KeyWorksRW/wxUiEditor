/////////////////////////////////////////////////////////////////////////////
// Purpose:   Add space to sizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/sizer.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_spacer_sizer.h"

bool SpacerGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    code.ParentName();

    if (node->getParent()->isGen(gen_wxGridBagSizer))
    {
        auto flags = node->getSizerFlags();

        code.Function("Add(").as_string(prop_width).Comma().as_string(prop_height);
        code.Comma().Object("wxGBPosition").as_string(prop_row).Comma().as_string(prop_column) += ")";
        code.Comma().Object("wxGBSpan").as_string(prop_rowspan).Comma().as_string(prop_colspan) += ")";
        code.Comma().itoa(flags.GetFlags()).Comma().as_string(prop_border_size);
        if (node->as_bool(prop_add_default_border))
        {
            code.Str(" + ").Add("wxSizerFlags").ClassMethod("GetDefaultBorder()");
        }
        code.EndFunction();
    }
    else
    {
        if (node->as_int(prop_proportion) != 0)
        {
            code.Function("AddStretchSpacer(").Add(prop_proportion).EndFunction();
        }
        else
        {
            if (node->as_int(prop_width) == node->as_int(prop_height))
            {
                code.Function("AddSpacer(").as_string(prop_width);
            }
            else if (node->getParent()->hasValue(prop_orientation))
            {
                code.Function("AddSpacer(");
                if (node->getParent()->as_string(prop_orientation) == "wxVERTICAL")
                {
                    code.as_string(prop_height);
                }
                else
                {
                    code.as_string(prop_width);
                }
            }

            else
            {
                code.Function("Add(").as_string(prop_width);
                if (node->as_bool(prop_add_default_border))
                {
                    code.Str(" + ").Add("wxSizerFlags").ClassMethod("GetDefaultBorder()");
                }
                code.Comma().as_string(prop_height);
            }

            if (node->as_bool(prop_add_default_border))
            {
                code.Str(" + ").Add("wxSizerFlags").ClassMethod("GetDefaultBorder()");
            }
            code.EndFunction();
        }
    }

    return true;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_spacer()

int SpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item = object;
    auto result = BaseGenerator::xrc_updated;

    item.append_attribute("class").set_value("spacer");
    item.append_child("size").text().set(tt_string() << node->as_string(prop_width) << ',' << node->as_string(prop_height));
    if (node->as_string(prop_proportion) != "0")
    {
        item.append_child("option").text().set(node->as_string(prop_proportion));
    }

    return result;
}

void SpacerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
