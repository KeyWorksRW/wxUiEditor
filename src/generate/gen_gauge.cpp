//////////////////////////////////////////////////////////////////////////
// Purpose:   wxGauge generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/gauge.h>  // wxGauge interface

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_gauge.h"

wxObject* GaugeGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxGauge(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_int(prop_range),
                              DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));
    widget->SetValue(node->as_int(prop_position));

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

bool GaugeGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma().as_string(prop_range);
    code.PosSizeFlags(true);

    return true;
}

bool GaugeGenerator::SettingsCode(Code& code)
{
    code.NodeName().Function("SetValue(").as_string(prop_position).EndFunction();

    return true;
}

bool GaugeGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/gauge.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

int GaugeGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
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
