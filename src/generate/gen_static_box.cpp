/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/statbox.h>

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "pugixml.hpp"  // xml read/write/create/process

#include "gen_static_box.h"

// This exists only for import compatability -- wxStaticBoxSizer is the preferred
// way to use this. It does, however, allow children.

wxObject* StaticBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxStaticBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_label),
                                  DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    return widget;
}

bool StaticBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().Add(prop_id).Comma().QuotedString(prop_label);
    code.PosSizeFlags();

    return true;
}

int StaticBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticBox");
    ADD_ITEM_PROP(prop_label, "label")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->as_bool(prop_markup))
        {
            item.append_child(pugi::node_comment).set_value(" markup cannot be be set in the XRC file. ");
        }

        GenXrcComments(node, item);
    }

    return result;
}

void StaticBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStaticBoxXmlHandler");
}

bool StaticBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statbox.h>", set_src, set_hdr);

    return true;
}
