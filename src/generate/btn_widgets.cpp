/////////////////////////////////////////////////////////////////////////////
// Purpose:   Button component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/button.h>  // wxButtonBase class
#include <wx/event.h>   // Event classes
#include <wx/tglbtn.h>  // wxToggleButtonBase

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "btn_widgets.h"

wxObject* ButtonGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxButton(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint("pos"),
                     node->prop_as_wxSize("size"), node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    if (node->prop_as_bool("markup"))
        widget->SetLabelMarkup(node->prop_as_wxString(txt_label));
    else
        widget->SetLabel(node->prop_as_wxString(txt_label));

    if (node->prop_as_bool("default"))
        widget->SetDefault();

    if (node->prop_as_bool("auth_needed"))
        widget->SetAuthNeeded();

    if (node->HasValue("bitmap"))
        widget->SetBitmap(node->prop_as_wxBitmap("bitmap"));

    if (node->HasValue("disabled_bmp"))
        widget->SetBitmapDisabled(node->prop_as_wxBitmap("disabled_bmp"));

    if (node->HasValue("pressed_bmp"))
        widget->SetBitmapPressed(node->prop_as_wxBitmap("pressed_bmp"));

    if (node->HasValue("focus"))
        widget->SetBitmapFocus(node->prop_as_wxBitmap("focus"));

    if (node->HasValue("current"))
        widget->SetBitmapCurrent(node->prop_as_wxBitmap("current"));

    if (node->HasValue("position"))
        widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int("position")));

    if (node->HasValue("margins"))
        widget->SetBitmapMargins(node->prop_as_wxSize("margins"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // We do not support changing the "markup" property because while the control displays correctly when markup is set,
    // it does not revert when markup is cleared (at least on Windows where markup controls whether a generic or native
    // version of the button is displayed).

    if (prop->GetPropName() == "label")
    {
        auto ctrl = wxStaticCast(widget, wxButton);
        if (node->prop_as_bool("markup"))
            ctrl->SetLabelMarkup(node->prop_as_wxString(txt_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(txt_label));

        return true;
    }
    else if (prop->GetPropName() == "markup")
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->prop_as_bool("markup"))
        {
            wxStaticCast(widget, wxButton)->SetLabelMarkup(node->prop_as_wxString(txt_label));
            return true;
        }
    }
    else if (prop->GetPropName() == "default")
    {
        // You can change a button to be the default, but you cannot change it back without recreating it.
        if (prop->as_bool())
        {
            wxStaticCast(widget, wxButton)->SetDefault();
            return true;
        }
    }

    return false;
}

std::optional<ttlib::cstr> ButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxButton(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(txt_label);
    if (label.size() && !node->prop_as_bool("markup"))
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code, true);
    if (code.contains("wxEmptyString)"))
        code.Replace(", wxEmptyString", "");

    return code;
}

std::optional<ttlib::cstr> ButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> ButtonGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool("markup"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(txt_label)) << ");";
    }

    if (node->prop_as_bool("default"))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetDefault();";
    }

    if (node->prop_as_bool("auth_needed"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetAuthNeeded();";
    }

    if (node->HasValue("bitmap"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string("bitmap")) << ");";
    }

    if (node->HasValue("disabled_bmp"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapDisabled(" << GenerateBitmapCode(node->prop_as_string("disabled_bmp"))
             << ");";
    }

    if (node->HasValue("pressed_bmp"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapPressed(" << GenerateBitmapCode(node->prop_as_string("pressed_bmp"))
             << ");";
    }

    if (node->HasValue("focus"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string("focus")) << ");";
    }

    if (node->HasValue("current"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string("current"))
             << ");";
    }

    return code;
}

bool ButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  ToggleButtonGenerator  //////////////////////////////////////////

wxObject* ToggleButtonGenerator::Create(Node* node, wxObject* parent)
{
    auto widget =
        new wxToggleButton(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint("pos"),
                           node->prop_as_wxSize("size"), node->prop_as_int(txt_style) | node->prop_as_int("window_style"));

    if (node->prop_as_bool("markup"))
        widget->SetLabelMarkup(node->prop_as_wxString(txt_label));
    else
        widget->SetLabel(node->prop_as_wxString(txt_label));

    widget->SetValue((node->prop_as_bool("pressed")));

    if (node->HasValue("bitmap"))
        widget->SetBitmap(node->prop_as_wxBitmap("bitmap"));

    if (node->HasValue("disabled_bmp"))
        widget->SetBitmapDisabled(node->prop_as_wxBitmap("disdisabled_bmp"));

    if (node->HasValue("pressed_bmp"))
        widget->SetBitmapPressed(node->prop_as_wxBitmap("pressed_bmp"));

    if (node->HasValue("focus"))
        widget->SetBitmapFocus(node->prop_as_wxBitmap("focus"));

    if (node->HasValue("current"))
        widget->SetBitmapCurrent(node->prop_as_wxBitmap("current"));

    if (node->HasValue("position"))
        widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int("position")));

    if (node->HasValue("margins"))
        widget->SetBitmapMargins(node->prop_as_wxSize("margins"));

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ToggleButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // We do not support changing the "markup" property because while the control displays correctly when markup is set,
    // it does not revert when markup is cleared (at least on Windows where markup controls whether a generic or native
    // version of the button is displayed).

    if (prop->GetPropName() == "label")
    {
        auto ctrl = wxStaticCast(widget, wxToggleButton);
        if (node->prop_as_bool("markup"))
            ctrl->SetLabelMarkup(node->prop_as_wxString(txt_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(txt_label));

        return true;
    }
    else if (prop->GetPropName() == "markup")
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->prop_as_bool("markup"))
        {
            wxStaticCast(widget, wxToggleButton)->SetLabelMarkup(node->prop_as_wxString(txt_label));
            return true;
        }
    }
    else if (prop->GetPropName() == "pressed")
    {
        wxStaticCast(widget, wxToggleButton)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> ToggleButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxToggleButton(";
    code << GetParentName(node) << ", " << node->prop_as_string("id") << ", ";

    auto& label = node->prop_as_string(txt_label);

    // If markup is true, then the label has to be set in the GenSettings() section
    if (label.size() && !node->prop_as_bool("markup"))
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code, true);
    if (code.contains("wxEmptyString)"))
        code.Replace(", wxEmptyString", "");

    return code;
}

std::optional<ttlib::cstr> ToggleButtonGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> ToggleButtonGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool("pressed"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetValue(true)";
    }

    if (node->prop_as_bool("markup"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(txt_label)) << ");";
    }

    if (node->HasValue("bitmap"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string("bitmap")) << ");";
    }

    if (node->HasValue("disabled_bmp"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapDisabled(" << GenerateBitmapCode(node->prop_as_string("disabled_bmp"))
             << ");";
    }

    if (node->HasValue("pressed_bmp"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapPressed(" << GenerateBitmapCode(node->prop_as_string("pressed_bmp"))
             << ");";
    }

    if (node->HasValue("focus"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string("focus")) << ");";
    }

    if (node->HasValue("current"))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string("current"))
             << ");";
    }

    return code;
}

bool ToggleButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/tglbtn.h>", set_src, set_hdr);
    return true;
}
