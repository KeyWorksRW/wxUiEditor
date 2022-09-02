//////////////////////////////////////////////////////////////////////////
// Purpose:   wxSpinButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/spinbutt.h>  // wxSpinButtonBase class

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_spin_btn.h"

wxObject* SpinButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinButton(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                   DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetRange(node->prop_as_int(prop_min), node->prop_as_int(prop_max));
    widget->SetValue(node->prop_as_int(prop_initial));

    if (node->prop_as_int(prop_inc) > 1)
        widget->SetIncrement(node->prop_as_int(prop_inc));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);
    GeneratePosSizeFlags(node, code, false, "wxSP_VERTICAL");

    code.Replace(", wxID_ANY);", ");");

    return code;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;
    code << node->get_node_name() << "->SetRange(" << node->prop_as_int(prop_min) << ", " << node->prop_as_int(prop_max)
         << ");";

    if (node->prop_as_int(prop_initial))
    {
        code << '\n' << node->get_node_name() << "->SetValue(" << node->prop_as_int(prop_initial) << ");";
    }

    if (node->prop_as_int(prop_inc) > 1)
    {
        code << '\n' << node->get_node_name() << "->SetIncrement(" << node->prop_as_int(prop_inc) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> SpinButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool SpinButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinbutt.h>", set_src, set_hdr);
    return true;
}

int SpinButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinButton");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")

    if (node->prop_as_int(prop_inc) > 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->HasValue(prop_style))
    {
        GenXrcStylePosSize(node, item);
    }
    else
    {
        // XRC is going to force the wxSP_ARROW_KEYS if we don't pass something. Since a spin control
        // can only be horizontal, we simply pass that flag.
        GenXrcPreStylePosSize(node, item, "wxSP_VERTICAL");
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void SpinButtonGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSpinButtonXmlHandler");
}
