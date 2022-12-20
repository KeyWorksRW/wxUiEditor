/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSpinCtrl and wxSpinCtrlDouble generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid
#include <wx/spinctrl.h>           // wxSpinCtrlBase class

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_spin_ctrl.h"

//////////////////////////////////////////  SpinCtrlGenerator  //////////////////////////////////////////

wxObject* SpinCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, DlgPoint(parent, node, prop_pos),
                                 DlgSize(parent, node, prop_size), GetStyleInt(node), node->prop_as_int(prop_min),
                                 node->prop_as_int(prop_max), node->prop_as_int(prop_initial));

    if (node->prop_as_bool(prop_hexadecimal))
        widget->SetBase(16);

    if (node->prop_as_int(prop_inc) > 1)
        widget->SetIncrement(node->prop_as_int(prop_inc));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool SpinCtrlGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_initial))
    {
        wxStaticCast(widget, wxSpinCtrl)->SetValue(node->prop_as_int(prop_initial));
        return true;
    }
    else if (prop->isProp(prop_min) || prop->isProp(prop_max))
    {
        wxStaticCast(widget, wxSpinCtrl)->SetRange(node->prop_as_int(prop_min), node->prop_as_int(prop_max));
        return true;
    }

    return false;
}

std::optional<ttlib::sview> SpinCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass().GetParentName();
    auto needed_parms = code.WhatParamsNeeded("wxSP_ARROW_KEYS");
    Node* node = code.node();
    if (needed_parms == Code::nothing_needed && node->as_int(prop_min) == 0 && node->as_int(prop_max) == 100 &&
        node->as_int(prop_initial) == 0)
    {
        if (node->as_string(prop_id) != "wxID_ANY")
            code.Comma().as_string(prop_id);
        code.EndFunction();
        return code.m_code;
    }
    code.Comma().as_string(prop_id).Comma().Add("wxEmptyString").Comma().Pos().Comma().WxSize().Comma().Style();
    code.Comma().as_string(prop_min).Comma().as_string(prop_max).Comma().as_string(prop_initial);
    if (needed_parms & Code::window_name_needed)
        code.Comma().QuotedString(prop_window_name);
    code.EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> SpinCtrlGenerator::CommonSettings(Code& code)
{
    if (code.IsTrue(prop_hexadecimal))
    {
        code.Eol(true).NodeName().Function("SetBase(16").EndFunction();
    }

    if (code.node()->as_int(prop_inc) > 1)
    {
        code.Eol(true).NodeName().Function("SetIncrement(").as_string(prop_inc).EndFunction();
    }

    return code.m_code;
}

int SpinCtrlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinCtrl");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")

    if (node->prop_as_int(prop_inc) > 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->prop_as_bool(prop_hexadecimal))
        item.append_child("base").text().set("16");

    if (node->HasValue(prop_style))
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
    return true;
}

//////////////////////////////////////////  SpinCtrlDoubleGenerator  //////////////////////////////////////////

wxObject* SpinCtrlDoubleGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSpinCtrlDouble(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node),
                                       node->prop_as_double(prop_min), node->prop_as_double(prop_max),
                                       node->prop_as_double(prop_initial), node->prop_as_double(prop_inc));

    if (node->prop_as_int(prop_digits) > 0)
    {
        widget->SetDigits(node->prop_as_int(prop_digits));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", wxEmptyString, ";

    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    GenStyle(node, code);
    code << ", " << node->prop_as_string(prop_min) << ", " << node->prop_as_string(prop_max) << ", "
         << node->prop_as_string(prop_initial) << ", " << node->prop_as_string(prop_inc);
    if (node->HasValue(prop_window_name))
    {
        code << ", " << node->prop_as_string(prop_window_name);
    }

    code << ");";

    code.Replace(", wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100, 0, 1);", ");");

    if (code.contains("wxEmptyString"))
    {
        code.Replace("wxEmptyString, ", "wxEmptyString,\n\t\t\t");
        code.insert(0, 1, '\t');
    }

    return code;
}

std::optional<ttlib::cstr> SpinCtrlDoubleGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_int(prop_digits) > 0)
    {
        code << node->get_node_name() << "->SetDigits(" << node->prop_as_string(prop_digits) << ");";
    }

    return code;
}

int SpinCtrlDoubleGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSpinCtrlDouble");

    ADD_ITEM_PROP(prop_min, "min")
    ADD_ITEM_PROP(prop_max, "max")
    ADD_ITEM_PROP(prop_initial, "value")
    ADD_ITEM_PROP(prop_digits, "digits")

    if (node->prop_as_double(prop_inc) != 1)
        ADD_ITEM_PROP(prop_inc, "inc")

    if (node->HasValue(prop_style))
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
