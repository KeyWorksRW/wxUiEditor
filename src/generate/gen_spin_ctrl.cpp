/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSpinCtrl and wxSpinCtrlDouble generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/spinctrl.h>           // wxSpinCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_spin_ctrl.h"

using namespace code;

//////////////////////////////////////////  SpinCtrlGenerator  //////////////////////////////////////////

wxObject* SpinCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node), node->as_int(prop_min),
                                 node->as_int(prop_max), node->as_int(prop_initial));

    if (node->as_bool(prop_hexadecimal))
        widget->SetBase(16);

    if (node->as_int(prop_inc) > 1)
        widget->SetIncrement(node->as_int(prop_inc));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SpinCtrlGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_initial))
    {
        wxStaticCast(widget, wxSpinCtrl)->SetValue(node->as_int(prop_initial));
        return true;
    }
    else if (prop->isProp(prop_min) || prop->isProp(prop_max))
    {
        wxStaticCast(widget, wxSpinCtrl)->SetRange(node->as_int(prop_min), node->as_int(prop_max));
        return true;
    }

    return false;
}

bool SpinCtrlGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName();
    auto needed_parms = code.WhatParamsNeeded("wxSP_ARROW_KEYS");
    Node* node = code.node();
    if (needed_parms == nothing_needed && node->as_int(prop_min) == 0 && node->as_int(prop_max) == 100 &&
        node->as_int(prop_initial) == 0)
    {
        if (node->as_string(prop_id) != "wxID_ANY")
            code.Comma().as_string(prop_id);
        code.EndFunction();
        return true;
    }
    code.Comma().as_string(prop_id).Comma().Add("wxEmptyString").Comma().Pos().Comma().WxSize().Comma().Style();
    code.Comma().itoa(prop_min, prop_max).Comma().as_string(prop_initial);
    if (needed_parms & window_name_needed)
        code.Comma().QuotedString(prop_window_name);
    code.EndFunction();

    return true;
}

bool SpinCtrlGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_hexadecimal))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetBase(16").EndFunction();
    }

    if (code.node()->as_int(prop_inc) > 1)
    {
        code.Eol(eol_if_empty).NodeName().Function("SetIncrement(").as_string(prop_inc).EndFunction();
    }

    return true;
}

int SpinCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinCtrl");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")

    if (node->as_int(prop_inc) > 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->as_bool(prop_hexadecimal))
        item.append_child("base").text().set("16");

    if (node->hasValue(prop_style))
    {
        GenXrcStylePosSize(node, item);
    }
    else
    {
        // XRC is going to force the wxSP_ARROW_KEYS if we don't pass something. Since a spin control
        // can only be horizontal, we simply pass that flag.
        GenXrcPreStylePosSize(node, item, "wxSP_HORIZONTAL");
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void SpinCtrlGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSpinCtrlXmlHandler");
}

bool SpinCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinctrl.h>", set_src, set_hdr);
    if (node->hasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

//////////////////////////////////////////  SpinCtrlDoubleGenerator  //////////////////////////////////////////

wxObject* SpinCtrlDoubleGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrlDouble(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_value),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node),
                                       node->as_double(prop_min), node->as_double(prop_max), node->as_double(prop_initial),
                                       node->as_double(prop_inc));

    if (node->as_int(prop_digits) > 0)
    {
        widget->SetDigits(node->as_int(prop_digits));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SpinCtrlDoubleGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName();
    auto needed_parms = code.WhatParamsNeeded("wxSP_ARROW_KEYS");
    Node* node = code.node();
    if (needed_parms == nothing_needed && node->as_int(prop_min) == 0 && node->as_int(prop_max) == 100 &&
        node->as_int(prop_initial) == 0 && node->as_int(prop_inc) == 1)
    {
        if (node->as_string(prop_id) != "wxID_ANY")
            code.Comma().as_string(prop_id);
        code.EndFunction();
        return true;
    }
    code.Comma().as_string(prop_id).Comma().Add("wxEmptyString").Comma().Pos().Comma().WxSize().Comma().Style();
    code.Comma().Str(prop_min).Comma().Str(prop_max).Comma().Str(prop_initial).Comma().Str(prop_inc);
    if (needed_parms & window_name_needed)
        code.Comma().QuotedString(prop_window_name);
    code.EndFunction();

    return true;
}

bool SpinCtrlDoubleGenerator::SettingsCode(Code& code)
{
    if (code.IntValue(prop_digits) > 0)
    {
        code.NodeName().Function("SetDigits(").Str(prop_digits).EndFunction();
    }

    return true;
}

int SpinCtrlDoubleGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinCtrlDouble");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")
    ADD_ITEM_PROP(prop_digits, "digits")

    if (node->as_double(prop_inc) != 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->hasValue(prop_style))
    {
        GenXrcStylePosSize(node, item);
    }
    else
    {
        // XRC is going to force the wxSP_ARROW_KEYS if we don't pass something. Since a spin control
        // can only be horizontal, we simply pass that flag.
        GenXrcPreStylePosSize(node, item, "wxSP_HORIZONTAL");
    }

    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void SpinCtrlDoubleGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSpinCtrlDoubleXmlHandler");
}

bool SpinCtrlDoubleGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/spinctrl.h>", set_src, set_hdr);
    if (node->hasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

bool SpinCtrlDoubleGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node)
{
    if (prop->isProp(prop_digits))
    {
        auto newValue = event->GetValue();
        if (newValue.GetInteger() > 20)
        {
            event->SetValidationFailureMessage("You can't specify more than 20 digits.");
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
