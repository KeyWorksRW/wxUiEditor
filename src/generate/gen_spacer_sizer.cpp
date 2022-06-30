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

std::optional<ttlib::cstr> SpacerGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << node->GetParent()->get_node_name();

    if (node->GetParent()->isGen(gen_wxGridBagSizer))
    {
        auto flags = node->GetSizerFlags();

        code << "->Add(" << node->prop_as_int(prop_width) << ", " << node->prop_as_int(prop_height);
        code << ", wxGBPosition(" << node->prop_as_int(prop_row) << ", " << node->prop_as_int(prop_column);
        code << "), wxGBSpan(" << node->prop_as_int(prop_rowspan) << ", " << node->prop_as_int(prop_colspan);
        code << "), " << flags.GetFlags() << ", " << node->prop_as_int(prop_border_size);
        if (node->prop_as_bool(prop_add_default_border))
            code << " + wxSizerFlags::GetDefaultBorder()";
        code << ");";
    }
    else
    {
        if (node->prop_as_int(prop_proportion) != 0)
        {
            code << "->AddStretchSpacer(" << node->prop_as_string(prop_proportion) << ");";
        }
        else
        {
            if (node->prop_as_int(prop_width) == node->prop_as_int(prop_height))
            {
                code << "->AddSpacer(" << node->prop_as_string(prop_width);
            }
            else if (node->GetParent()->HasValue(prop_orientation))
            {
                code << "->AddSpacer(";
                if (node->GetParent()->prop_as_string(prop_orientation) == "wxVERTICAL")
                {
                    code << node->prop_as_string(prop_height);
                }
                else
                {
                    code << node->prop_as_string(prop_width);
                }
            }

            else
            {
                code << "->Add(" << node->prop_as_string(prop_width);
                if (node->prop_as_bool(prop_add_default_border))
                    code << " + wxSizerFlags::GetDefaultBorder()";
                code << ", " << node->prop_as_string(prop_height);
            }

            if (node->prop_as_bool(prop_add_default_border))
                code << " + wxSizerFlags::GetDefaultBorder()";

            code << ");";
        }
    }

    return code;
}

// ../../wxSnapShot/src/xrc/xh_sizer.cpp
// ../../../wxWidgets/src/xrc/xh_sizer.cpp
// See Handle_spacer()

int SpacerGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    pugi::xml_node item = object;
    auto result = BaseGenerator::xrc_updated;

    item.append_attribute("class").set_value("spacer");
    item.append_child("size").text().set(ttlib::cstr()
                                         << node->prop_as_string(prop_width) << ',' << node->prop_as_string(prop_height));
    if (node->prop_as_string(prop_proportion) != "0")
    {
        item.append_child("option").text().set(node->prop_as_string(prop_proportion));
    }

    return result;
}

void SpacerGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSizerXmlHandler");
}
