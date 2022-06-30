//////////////////////////////////////////////////////////////////////////
// Purpose:   wxScrollBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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

    widget->SetScrollbar(node->prop_as_int(prop_position), node->prop_as_int(prop_thumbsize), node->prop_as_int(prop_range),
                         node->prop_as_int(prop_pagesize));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ScrollBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    code << '\t';  // lead with tab since we add a second line
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code);

    code << "\n\t" << node->get_node_name() << "->SetScrollbar(" << node->prop_as_string(prop_position);
    code << ", " << node->prop_as_string(prop_thumbsize) << ", " << node->prop_as_string(prop_range);
    code << ", " << node->prop_as_string(prop_pagesize) << ");";

    return code;
}

std::optional<ttlib::cstr> ScrollBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
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
