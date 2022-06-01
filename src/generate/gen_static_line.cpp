//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStaticLine generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/statline.h>  // wxStaticLine class interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_static_line.h"

wxObject* StaticLineGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxStaticLine(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StaticLineGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxStaticLine(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_style) != "wxLI_HORIZONTAL")
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        auto pos = node->prop_as_wxPoint(prop_pos);
        auto size = node->prop_as_wxPoint(prop_size);
        auto& win_name = node->prop_as_string(prop_window_name);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (win_name.empty() && win_style.empty() && size.x == -1 && size.y == -1 && pos.x == -1 && pos.y == -1)
        {
            code << ");";
            // A lot easier to remove the id once we get here then to add the logic above to not add it
            code.Replace(", wxID_ANY", "");
            return code;
        }

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
        GenStyle(node, code);
    }

    return code;
}

bool StaticLineGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statline.h>", set_src, set_hdr);
    return true;
}

int StaticLineGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStaticLine");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void StaticLineGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStaticLineXmlHandler");
}
