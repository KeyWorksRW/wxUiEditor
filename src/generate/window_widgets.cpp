/////////////////////////////////////////////////////////////////////////////
// Purpose:   Splitter and Scroll window component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/scrolwin.h>  // wxScrolledWindow, wxScrolledControl and wxScrollHelper

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "window_widgets.h"

//////////////////////////////////////////  ScrolledCanvasGenerator  //////////////////////////////////////////

wxObject* ScrolledCanvasGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrolled<wxWindow>(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                           DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetScrollRate(node->prop_as_int(prop_scroll_rate_x), node->prop_as_int(prop_scroll_rate_y));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ScrolledCanvasGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxScrolled<wxWindow>(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ScrolledCanvasGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_scroll_rate_x) || node->HasValue(prop_scroll_rate_y))
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetScrollRate(" << node->prop_as_string(prop_scroll_rate_x) << ", "
             << node->prop_as_string(prop_scroll_rate_y) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ScrolledCanvasGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ScrolledCanvasGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/scrolwin.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  ScrolledWindowGenerator  //////////////////////////////////////////

wxObject* ScrolledWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrolled<wxPanel>(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetScrollRate(node->prop_as_int(prop_scroll_rate_x), node->prop_as_int(prop_scroll_rate_y));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";

    code << node->get_node_name() << " = new wxScrolled<wxPanel>(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_scroll_rate_x) || node->HasValue(prop_scroll_rate_y))
    {
        if (code.size())
            code << "\n";

        code << node->get_node_name() << "->SetScrollRate(" << node->prop_as_string(prop_scroll_rate_x) << ", "
             << node->prop_as_string(prop_scroll_rate_y) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ScrolledWindowGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ScrolledWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/scrolwin.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int ScrolledWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxScrolledWindow");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (node->as_int(prop_scroll_rate_x) >= 0 || node->as_int(prop_scroll_rate_y) >= 0)
    {
        ttlib::cstr scroll_rate;
        scroll_rate << node->as_int(prop_scroll_rate_x) << ',' << node->as_int(prop_scroll_rate_y);
        item.append_child("scrollrate").text().set(scroll_rate);
    }

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ScrolledWindowGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxScrolledWindowXmlHandler");
}
