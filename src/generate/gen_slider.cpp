//////////////////////////////////////////////////////////////////////////
// Purpose:   wxSlider generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/slider.h>             // wxSlider interface

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_slider.h"

wxObject* SliderGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSlider(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_int(prop_value),
                               node->as_int(prop_minValue), node->as_int(prop_maxValue), DlgPoint(parent, node, prop_pos),
                               DlgSize(parent, node, prop_size), GetStyleInt(node));

    widget->SetValue(node->as_int(prop_position));
    if (node->as_int(prop_line_size) > 0)
        widget->SetLineSize(node->as_int(prop_line_size));
    if (node->as_int(prop_page_size) > 0)
        widget->SetPageSize(node->as_int(prop_page_size));
#if defined(_WIN32)
    if (node->as_int(prop_tick_frequency) > 0)
        widget->SetTickFreq(node->as_int(prop_tick_frequency));
    if (node->as_int(prop_thumb_length) > 0)
        widget->SetThumbLength(node->as_int(prop_thumb_length));
    if (node->as_int(prop_sel_start) >= 0 && node->as_int(prop_sel_end) >= 0)
    {
        widget->SetSelection(node->as_int(prop_sel_start), node->as_int(prop_sel_end));
    }
#endif  // _WIN32

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SliderGenerator::OnPropertyChange(wxObject* widget, Node* /* node */, NodeProperty* prop)
{
    if (prop->isProp(prop_position))
    {
        wxStaticCast(widget, wxSlider)->SetValue(prop->as_int());
        return true;
    }

    return false;
}

bool SliderGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();
    code.as_string(prop_position).Comma().as_string(prop_minValue).Comma().as_string(prop_maxValue);
    code.PosSizeFlags(true);

    return true;
}

bool SliderGenerator::SettingsCode(Code& code)
{
    code.NodeName().Function("SetValue(").as_string(prop_position).EndFunction();

    Node* node = code.node();
    if (node->as_int(prop_sel_start) >= 0 && node->as_int(prop_sel_end) >= 0)
    {
        code.Eol(eol_if_empty)
            .NodeName()
            .Function("SetSelection(")
            .as_string(prop_sel_start)
            .Comma()
            .as_string(prop_sel_end)
            .EndFunction();
    }
    if (node->as_int(prop_line_size) > 0)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetLineSize(").as_string(prop_line_size).EndFunction();
    }
    if (node->as_int(prop_page_size) > 0)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetPageSize(").as_string(prop_page_size).EndFunction();
    }
    if (node->as_int(prop_tick_frequency) > 0)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetTickFreq(").as_string(prop_tick_frequency).EndFunction();
    }
    if (node->as_int(prop_thumb_length) > 0)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetThumbLength(").as_string(prop_thumb_length).EndFunction();
    }

    return true;
}

bool SliderGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_minValue))
    {
        auto newValue = event->GetValue();
        if (newValue.GetInteger() > node->as_int(prop_maxValue))
        {
            event->SetValidationFailureMessage("Slider minimum must be less than the maximum.");
            event->Veto();
            return false;
        }
        return true;
    }
    else if (prop->isProp(prop_maxValue))
    {
        auto newValue = event->GetValue();
        if (newValue.GetInteger() < node->as_int(prop_minValue))
        {
            event->SetValidationFailureMessage("Slider maximum must be greater than the minimum.");
            event->Veto();
            return false;
        }
        return true;
    }
    else
    {
        return BaseGenerator::AllowPropertyChange(event, prop, node);
    }
}

bool SliderGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/slider.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

int SliderGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSlider");

    ADD_ITEM_PROP(prop_position, "value")
    ADD_ITEM_PROP(prop_minValue, "min")
    ADD_ITEM_PROP(prop_maxValue, "max")
    ADD_ITEM_PROP(prop_tick_frequency, "tickfreq")
    ADD_ITEM_PROP(prop_page_size, "pagesize")
    ADD_ITEM_PROP(prop_line_size, "linesize")
    ADD_ITEM_PROP(prop_thumb_length, "thumb")

    if (node->as_int(prop_sel_start) >= 0 && node->as_int(prop_sel_end) >= 0)
    {
        ADD_ITEM_PROP(prop_sel_start, "selmin")
        ADD_ITEM_PROP(prop_sel_end, "selmax")
    }

    GenXrcStylePosSize(node, item, prop_orientation);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void SliderGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSliderXmlHandler");
}
