/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCheckBox component class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/checkbox.h>  // wxCheckBox class interface
#include <wx/checklst.h>  // wxCheckListBox class interface
#include <wx/event.h>     // Event classes

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "checkbox_widgets.h"

wxObject* CheckBoxGenerator::Create(Node* node, wxObject* parent)
{
    long style_value = 0;
    if (node->prop_as_string(prop_style).contains("wxALIGN_RIGHT"))
        style_value |= wxALIGN_RIGHT;

    auto widget = new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                 node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size),
                                 style_value | node->prop_as_int(prop_window_style));

    if (node->prop_as_bool(prop_checked))
        widget->SetValue(true);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool CheckBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxCheckBox)->SetLabel(node->prop_as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_checked))
    {
        wxStaticCast(widget, wxCheckBox)->SetValue(prop->as_bool());
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> CheckBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCheckBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    GeneratePosSizeFlags(node, code);

    return code;
}

std::optional<ttlib::cstr> CheckBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> CheckBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_checked))
        code << node->get_node_name() << "->SetValue(true);";

    return code;
}

bool CheckBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checkbox.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  Check3StateGenerator  //////////////////////////////////////////

wxObject* Check3StateGenerator::Create(Node* node, wxObject* parent)
{
    long style_value = wxCHK_3STATE | node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style);

    auto widget = new wxCheckBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_label),
                                 node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size), style_value);

    auto& state = node->prop_as_string(prop_initial_state);
    if (state == "wxCHK_UNCHECKED")
        widget->Set3StateValue(wxCHK_UNCHECKED);
    else if (state == "wxCHK_CHECKED")
        widget->Set3StateValue(wxCHK_CHECKED);
    else
        widget->Set3StateValue(wxCHK_UNDETERMINED);

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool Check3StateGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (prop->isProp(prop_label))
    {
        wxStaticCast(widget, wxCheckBox)->SetLabel(node->prop_as_wxString(prop_label));
        return true;
    }
    else if (prop->isProp(prop_initial_state))
    {
        auto& state = prop->as_string();
        if (state == "wxCHK_UNCHECKED")
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_UNCHECKED);
        else if (state == "wxCHK_CHECKED")
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_CHECKED);
        else
            wxStaticCast(widget, wxCheckBox)->Set3StateValue(wxCHK_UNDETERMINED);
        return true;
    }

    return false;
}

std::optional<ttlib::cstr> Check3StateGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCheckBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    auto& label = node->prop_as_string(prop_label);
    if (label.size())
    {
        code << GenerateQuotedString(label);
    }
    else
    {
        code << "wxEmptyString";
    }

    code << ", ";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", ";
    code << "wxCHK_3STATE";
    auto& style = node->prop_as_string(prop_style);
    if (style.size())
        code << '|' << style;
    auto& win_style = node->prop_as_string(prop_window_style);
    if (win_style.size())
        code << '|' << win_style;
    auto& win_name = node->prop_as_string(prop_window_name);
    if (win_name.size())
    {
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
    }
    code << ");";

    return code;
}

std::optional<ttlib::cstr> Check3StateGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

std::optional<ttlib::cstr> Check3StateGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    auto& state = node->prop_as_string(prop_initial_state);
    if (state == "wxCHK_CHECKED")
    {
        code << node->get_node_name() << "->Set3StateValue(wxCHK_CHECKED);";
    }
    else if (state == "wxCHK_UNDETERMINED")
    {
        code << node->get_node_name() << "->Set3StateValue(wxCHK_UNDETERMINED);";
    }

    return code;
}

bool Check3StateGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checkbox.h>", set_src, set_hdr);
    return true;
}
