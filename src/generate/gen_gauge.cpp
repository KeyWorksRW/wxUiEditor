//////////////////////////////////////////////////////////////////////////
// Purpose:   wxGauge generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/gauge.h>  // wxGauge interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_gauge.h"

wxObject* GaugeGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGauge(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_int(prop_range),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetValue(node->prop_as_int(prop_position));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool GaugeGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->isProp(prop_position))
    {
        wxStaticCast(widget, wxGauge)->SetValue(prop->as_int());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> GaugeGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", " << node->prop_as_string(prop_range);

    auto& win_name = node->prop_as_string(prop_window_name);
    if (win_name.size())
    {
        // Window name is always the last parameter, so if it is specified, everything has to be generated.
        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", ";
    }

    GeneratePosSizeFlags(node, code, true, "wxGA_HORIZONTAL");

    return code;
}

std::optional<ttlib::cstr> GaugeGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    // If a validator has been specified, then the variable will be initialized in the header file.
    if (node->prop_as_string(prop_validator_variable).empty())
    {
        code << node->get_node_name() << "->SetValue(" << node->prop_as_string(prop_position) << ");";
    }

    return code;
}

std::optional<ttlib::cstr> GaugeGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool GaugeGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/gauge.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

int GaugeGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxGauge");

    ADD_ITEM_PROP(prop_range, "range")
    ADD_ITEM_PROP(prop_position, "value")

    GenXrcStylePosSize(node, item, prop_orientation);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void GaugeGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxGauge");
}
