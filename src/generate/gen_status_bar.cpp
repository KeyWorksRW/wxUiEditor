//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStatusBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/statusbr.h>  // wxStatusBar class interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_status_bar.h"

wxObject* StatusBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto org_style = GetStyleInt(node);
    // Don't display the gripper as it can resize our main window rather than just the mockup window
    auto widget = new wxStatusBar(wxStaticCast(parent, wxWindow), wxID_ANY, (org_style &= ~wxSTB_SIZEGRIP));
    widget->SetFieldsCount(node->prop_as_int(prop_fields));

    if (org_style & wxSTB_SIZEGRIP)
        widget->SetStatusText("gripper not displayed in Mock Up");

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = CreateStatusBar(";

    if (node->prop_as_string(prop_window_name).size())
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
        code << ", " << node->prop_as_string(prop_window_name);
    }
    else if (node->prop_as_int(prop_style) != wxSTB_DEFAULT_STYLE || node->prop_as_int(prop_window_style) > 0)
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
    }
    else if (node->prop_as_string(prop_id) != "wxID_ANY")
    {
        code << node->prop_as_int(prop_fields) << ", " << node->prop_as_string(prop_id);
    }
    else if (node->prop_as_int(prop_fields) > 1)
    {
        code << node->prop_as_int(prop_fields);
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool StatusBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statusbr.h>", set_src, set_hdr);
    return true;
}

int StatusBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStatusBar");

    ADD_ITEM_PROP(prop_fields, "fields")

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void StatusBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStatusBarXmlHandler");
}
