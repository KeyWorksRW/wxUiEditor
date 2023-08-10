//////////////////////////////////////////////////////////////////////////
// Purpose:   wxToggleButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/tglbtn.h>  // This dummy header includes the proper header file for the

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_toggle_btn.h"

wxObject* ToggleButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToggleButton(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                     DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), GetStyleInt(node));

    if (node->as_bool(prop_markup))
        widget->SetLabelMarkup(node->as_wxString(prop_label));
    else
        widget->SetLabel(node->as_wxString(prop_label));

    widget->SetValue((node->as_bool(prop_pressed)));

    if (node->hasValue(prop_bitmap))
        widget->SetBitmap(node->as_wxBitmapBundle(prop_bitmap));

    if (node->hasValue(prop_disabled_bmp))
        widget->SetBitmapDisabled(node->as_wxBitmapBundle(prop_disabled_bmp));

    if (node->hasValue(prop_pressed_bmp))
        widget->SetBitmapPressed(node->as_wxBitmapBundle(prop_pressed_bmp));

    if (node->hasValue(prop_focus_bmp))
        widget->SetBitmapFocus(node->as_wxBitmapBundle(prop_focus_bmp));

    if (node->hasValue(prop_current))
        widget->SetBitmapCurrent(node->as_wxBitmapBundle(prop_current));

    if (node->hasValue(prop_position))
        widget->SetBitmapPosition(static_cast<wxDirection>(node->as_int(prop_position)));

    if (node->hasValue(prop_margins))
        widget->SetBitmapMargins(node->as_wxSize(prop_margins));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ToggleButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // We do not support changing the "markup" property because while the control displays correctly when markup is set,
    // it does not revert when markup is cleared (at least on Windows where markup controls whether a generic or native
    // version of the button is displayed).

    if (prop->isProp(prop_label))
    {
        auto ctrl = wxStaticCast(widget, wxToggleButton);
        if (node->as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->as_wxString(prop_label));
        else
            ctrl->SetLabel(node->as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->as_bool(prop_markup))
        {
            wxStaticCast(widget, wxToggleButton)->SetLabelMarkup(node->as_wxString(prop_label));
            return true;
        }
    }
    else if (prop->isProp(prop_pressed))
    {
        wxStaticCast(widget, wxToggleButton)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

bool ToggleButtonGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id).Comma();

    // If prop_markup is set, then the label will be set in SettingsCode()
    if (code.hasValue(prop_label) && !code.IsTrue(prop_markup))
    {
        code.QuotedString(prop_label);
    }
    else
    {
        code.Add("wxEmptyString");
    }

    code.PosSizeFlags(true);

    return true;
}

bool ToggleButtonGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_pressed))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetValue(").True().EndFunction();
    }

    if (code.IsTrue(prop_markup) && code.hasValue(prop_label))
    {
        code.Eol(eol_if_needed).NodeName().Function("SetLabelMarkup(").QuotedString(prop_label).EndFunction();
    }

    if (code.hasValue(prop_bitmap))
    {
        if (code.hasValue(prop_position))
        {
            code.Eol(eol_if_needed).NodeName().Function("SetBitmapPosition(").as_string(prop_position).EndFunction();
        }

        if (code.hasValue(prop_margins))
        {
            auto size = code.node()->as_wxSize(prop_margins);
            code.Eol(eol_if_needed).NodeName().Function("SetBitmapMargins(");
            code.itoa(size.GetWidth()).Comma().itoa(size.GetHeight()).EndFunction();
        }

        if (code.is_cpp())
            GenBtnBimapCode(code.node(), code.GetCode());
        else
            PythonBtnBimapCode(code);
    }

    return true;
}

bool ToggleButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/tglbtn.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

int ToggleButtonGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, node->hasValue(prop_label) ? "wxToggleButton" : "wxBitmapToggleButton");

    ADD_ITEM_PROP(prop_label, "label")
    ADD_ITEM_BOOL(prop_pressed, "checked")

    GenXrcBitmap(node, item, xrc_flags);
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ToggleButtonGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxToggleButtonXmlHandler");
}
