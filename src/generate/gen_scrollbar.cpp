//////////////////////////////////////////////////////////////////////////
// Purpose:   wxScrollBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/scrolbar.h>  // wxScrollBar base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_scrollbar.h"

wxObject* ScrollBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrollBar(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                  DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetScrollbar(node->as_int(prop_position), node->as_int(prop_thumbsize), node->as_int(prop_range),
                         node->as_int(prop_pagesize));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ScrollBarGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    code.PosSizeFlags();

    return true;
}

bool ScrollBarGenerator::SettingsCode(Code& code)
{
    code.NodeName().Function("SetScrollbar(").Str(prop_position).Comma().Str(prop_thumbsize);
    code.Comma().Str(prop_range).Comma().Str(prop_pagesize).EndFunction();
    return true;
}

int ScrollBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxScrollBar");

    ADD_ITEM_PROP(prop_position, "value")
    ADD_ITEM_PROP(prop_pagesize, "pagesize")
    ADD_ITEM_PROP(prop_range, "range")
    ADD_ITEM_PROP(prop_thumbsize, "thumbsize")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ScrollBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxScrollBarXmlHandler");
}

bool ScrollBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/scrolbar.h>", set_src, set_hdr);
    return true;
}
