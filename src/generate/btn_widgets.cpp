/////////////////////////////////////////////////////////////////////////////
// Purpose:   Button component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/button.h>             // wxButtonBase class
#include <wx/commandlinkbutton.h>  // wxCommandLinkButtonBase and wxGenericCommandLinkButton classes
#include <wx/event.h>              // Event classes
#include <wx/tglbtn.h>             // wxToggleButtonBase

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "btn_widgets.h"

using namespace GenEnum;

//////////////////////////////////////////  ButtonGenerator  //////////////////////////////////////////

wxObject* ButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxButton(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, node->prop_as_wxPoint(prop_pos),
                     node->prop_as_wxSize(prop_size), node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->prop_as_bool(prop_markup))
        widget->SetLabelMarkup(node->prop_as_wxString(prop_label));
    else
        widget->SetLabel(node->prop_as_wxString(prop_label));

    if (node->prop_as_bool(prop_default))
        widget->SetDefault();

    if (node->prop_as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmap(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->prop_as_wxBitmap(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->prop_as_wxBitmap(prop_pressed_bmp));

        if (node->HasValue(prop_focus))
            widget->SetBitmapFocus(node->prop_as_wxBitmap(prop_focus));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->prop_as_wxBitmap(prop_current));

        if (node->HasValue(prop_position))
            widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int(prop_position)));

        if (node->HasValue(prop_margins))
            widget->SetBitmapMargins(node->prop_as_wxSize(prop_margins));
    }

    if (!node->isPropValue(prop_variant, "normal"))
    {
        if (node->isPropValue(prop_variant, "small"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
        else if (node->isPropValue(prop_variant, "mini"))
            widget->SetWindowVariant(wxWINDOW_VARIANT_MINI);
        else
            widget->SetWindowVariant(wxWINDOW_VARIANT_LARGE);
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ButtonGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    // We do not support changing the "markup" property because while the control displays correctly when markup is set,
    // it does not revert when markup is cleared (at least on Windows where markup controls whether a generic or native
    // version of the button is displayed).

    if (prop->isProp(prop_label))
    {
        auto ctrl = wxStaticCast(widget, wxButton);
        if (node->prop_as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->prop_as_bool(prop_markup))
        {
            wxStaticCast(widget, wxButton)->SetLabelMarkup(node->prop_as_wxString(prop_label));
            return true;
        }
    }
    else if (prop->isProp(prop_default))
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
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (!node->prop_as_bool(prop_markup))
    {
        code << GenerateQuotedString(node, prop_label);
    }
    else
    {
        // prop_markup is set, so the actual label will be set in GenSettings()
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

    if (node->prop_as_bool(prop_markup))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(prop_label))
             << ");";
    }

    if (node->prop_as_bool(prop_default))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetDefault();";
    }

    if (node->prop_as_bool(prop_auth_needed))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetAuthNeeded();";
    }

    if (node->HasValue(prop_bitmap))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";

        if (node->HasValue(prop_position))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPosition(" << node->prop_as_int(prop_position) << ");";
        }

        if (node->HasValue(prop_margins))
        {
            if (code.size())
                code << '\n';
            auto size = node->prop_as_wxSize(prop_margins);
            code << node->get_node_name() << "->SetBitmapMargins(" << size.GetWidth() << ", " << size.GetHeight() << ");";
        }

        if (node->HasValue(prop_disabled_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapDisabled("
                 << GenerateBitmapCode(node->prop_as_string(prop_disabled_bmp)) << ");";
        }

        if (node->HasValue(prop_pressed_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPressed("
                 << GenerateBitmapCode(node->prop_as_string(prop_pressed_bmp)) << ");";
        }

        if (node->HasValue(prop_focus))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string(prop_focus))
                 << ");";
        }

        if (node->HasValue(prop_current))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string(prop_current))
                 << ");";
        }
    }
    return code;
}

bool ButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/button.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  ToggleButtonGenerator  //////////////////////////////////////////

wxObject* ToggleButtonGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxToggleButton(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                     node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                                     node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    if (node->prop_as_bool(prop_markup))
        widget->SetLabelMarkup(node->prop_as_wxString(prop_label));
    else
        widget->SetLabel(node->prop_as_wxString(prop_label));

    widget->SetValue((node->prop_as_bool(prop_pressed)));

    if (node->HasValue(prop_bitmap))
        widget->SetBitmap(node->prop_as_wxBitmap(prop_bitmap));

    if (node->HasValue(prop_disabled_bmp))
        widget->SetBitmapDisabled(node->prop_as_wxBitmap(prop_disabled_bmp));

    if (node->HasValue(prop_pressed_bmp))
        widget->SetBitmapPressed(node->prop_as_wxBitmap(prop_pressed_bmp));

    if (node->HasValue(prop_focus))
        widget->SetBitmapFocus(node->prop_as_wxBitmap(prop_focus));

    if (node->HasValue(prop_current))
        widget->SetBitmapCurrent(node->prop_as_wxBitmap(prop_current));

    if (node->HasValue(prop_position))
        widget->SetBitmapPosition(static_cast<wxDirection>(node->prop_as_int(prop_position)));

    if (node->HasValue(prop_margins))
        widget->SetBitmapMargins(node->prop_as_wxSize(prop_margins));

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
        if (node->prop_as_bool(prop_markup))
            ctrl->SetLabelMarkup(node->prop_as_wxString(prop_label));
        else
            ctrl->SetLabel(node->prop_as_wxString(prop_label));

        return true;
    }
    else if (prop->isProp(prop_markup))
    {
        // Turning markup on switches to generic rending of the button. However, you have to recreate it to switch it
        // off and go back to native rendering.

        if (node->prop_as_bool(prop_markup))
        {
            wxStaticCast(widget, wxToggleButton)->SetLabelMarkup(node->prop_as_wxString(prop_label));
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

std::optional<ttlib::cstr> ToggleButtonGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxToggleButton(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    if (!node->prop_as_bool(prop_markup))
    {
        code << GenerateQuotedString(node, prop_label);
    }
    else
    {
        // prop_markup is set, so the actual label will be set in GenSettings()
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

    if (node->prop_as_bool(prop_pressed))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetValue(true)";
    }

    if (node->prop_as_bool(prop_markup))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetLabelMarkup(" << GenerateQuotedString(node->prop_as_string(prop_label))
             << ");";
    }

    if (node->HasValue(prop_bitmap))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";

        if (node->HasValue(prop_position))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPosition(" << node->prop_as_int(prop_position) << ");";
        }

        if (node->HasValue(prop_margins))
        {
            if (code.size())
                code << '\n';
            auto size = node->prop_as_wxSize(prop_margins);
            code << node->get_node_name() << "->SetBitmapMargins(" << size.GetWidth() << ", " << size.GetHeight() << ");";
        }

        if (node->HasValue(prop_disabled_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapDisabled("
                 << GenerateBitmapCode(node->prop_as_string(prop_disabled_bmp)) << ");";
        }

        if (node->HasValue(prop_pressed_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPressed("
                 << GenerateBitmapCode(node->prop_as_string(prop_pressed_bmp)) << ");";
        }

        if (node->HasValue(prop_focus))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string(prop_focus))
                 << ");";
        }

        if (node->HasValue(prop_current))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string(prop_current))
                 << ");";
        }
    }

    return code;
}

bool ToggleButtonGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/tglbtn.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  CommandLinkBtnGenerator  //////////////////////////////////////////

wxObject* CommandLinkBtnGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxCommandLinkButton(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_main_label),
                                          node->prop_as_wxString(prop_note), node->prop_as_wxPoint(prop_pos),
                                          node->prop_as_wxSize(prop_size), node->prop_as_int(prop_window_style));

    if (node->prop_as_bool(prop_default))
        widget->SetDefault();

    if (node->prop_as_bool(prop_auth_needed))
        widget->SetAuthNeeded();

    if (node->HasValue(prop_bitmap))
    {
        widget->SetBitmap(node->prop_as_wxBitmap(prop_bitmap));

        if (node->HasValue(prop_disabled_bmp))
            widget->SetBitmapDisabled(node->prop_as_wxBitmap(prop_disabled_bmp));

        if (node->HasValue(prop_pressed_bmp))
            widget->SetBitmapPressed(node->prop_as_wxBitmap(prop_pressed_bmp));

        if (node->HasValue(prop_focus))
            widget->SetBitmapFocus(node->prop_as_wxBitmap(prop_focus));

        if (node->HasValue(prop_current))
            widget->SetBitmapCurrent(node->prop_as_wxBitmap(prop_current));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCommandLinkButton(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    code << GenerateQuotedString(node, prop_main_label) << ", " << GenerateQuotedString(node, prop_note);

    GeneratePosSizeFlags(node, code, true);

    return code;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_default))
    {
        if (code.size())
            code << '\n';

        code << node->get_node_name() << "->SetDefault();";
    }

    if (node->prop_as_bool(prop_auth_needed))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetAuthNeeded();";
    }

    if (node->HasValue(prop_bitmap))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetBitmap(" << GenerateBitmapCode(node->prop_as_string(prop_bitmap)) << ");";

        if (node->HasValue(prop_disabled_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapDisabled("
                 << GenerateBitmapCode(node->prop_as_string(prop_disabled_bmp)) << ");";
        }

        if (node->HasValue(prop_pressed_bmp))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapPressed("
                 << GenerateBitmapCode(node->prop_as_string(prop_pressed_bmp)) << ");";
        }

        if (node->HasValue(prop_focus))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapFocus(" << GenerateBitmapCode(node->prop_as_string(prop_focus))
                 << ");";
        }

        if (node->HasValue(prop_current))
        {
            if (code.size())
                code << '\n';
            code << node->get_node_name() << "->SetBitmapCurrent(" << GenerateBitmapCode(node->prop_as_string(prop_current))
                 << ");";
        }
    }

    return code;
}

std::optional<ttlib::cstr> CommandLinkBtnGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CommandLinkBtnGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/commandlinkbutton.h>", set_src, set_hdr);
    return true;
}
