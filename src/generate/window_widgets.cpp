/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scroll window component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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
    widget->SetScrollRate(node->as_int(prop_scroll_rate_x), node->as_int(prop_scroll_rate_y));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ScrolledCanvasGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.AddAuto().NodeName().Str(" = new wxScrolled<wxWindow>(");
        code.ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();
    }
    else
    {
        code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();
    }

    return true;
}

bool ScrolledCanvasGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_scroll_rate_x) || code.hasValue(prop_scroll_rate_y))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetScrollRate(");
        code.as_string(prop_scroll_rate_x).Comma().as_string(prop_scroll_rate_y).EndFunction();
    }

    return true;
}

bool ScrolledCanvasGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/scrolwin.h>", set_src, set_hdr);

    return true;
}

//////////////////////////////////////////  ScrolledWindowGenerator  //////////////////////////////////////////

wxObject* ScrolledWindowGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxScrolled<wxPanel>(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetScrollRate(node->as_int(prop_scroll_rate_x), node->as_int(prop_scroll_rate_y));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ScrolledWindowGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp())
    {
        code.AddAuto().NodeName().Str(" = new wxScrolled<wxPanel>(");
        code.ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();
    }
    else
    {
        code.NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
        code.PosSizeFlags();
    }

    return true;
}

bool ScrolledWindowGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_scroll_rate_x) || code.hasValue(prop_scroll_rate_y))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetScrollRate(");
        code.as_string(prop_scroll_rate_x).Comma().as_string(prop_scroll_rate_y).EndFunction();
    }

    return true;
}

bool ScrolledWindowGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/scrolwin.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_wizrd.cpp
// ../../../wxWidgets/src/xrc/xh_wizrd.cpp

int ScrolledWindowGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxScrolledWindow");

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (node->as_int(prop_scroll_rate_x) >= 0 || node->as_int(prop_scroll_rate_y) >= 0)
    {
        tt_string scroll_rate;
        scroll_rate << node->as_int(prop_scroll_rate_x) << ',' << node->as_int(prop_scroll_rate_y);
        item.append_child("scrollrate").text().set(scroll_rate);
    }

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ScrolledWindowGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxScrolledWindowXmlHandler");
}
