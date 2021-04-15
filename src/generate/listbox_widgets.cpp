/////////////////////////////////////////////////////////////////////////////
// Purpose:   ListBox component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/checklst.h>       // wxCheckListBox class interface
#include <wx/event.h>          // Event classes
#include <wx/htmllbox.h>       // wxHtmlListBox is a listbox whose items are wxHtmlCells
#include <wx/listbox.h>        // wxListBox class interface
#include <wx/rearrangectrl.h>  // various controls for rearranging the items interactively

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "listbox_widgets.h"

//////////////////////////////////////////  ListBoxGenerator  //////////////////////////////////////////

wxObject* ListBoxGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxListBox(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size), 0,
        nullptr, node->prop_as_int(prop_type) | node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    auto& items = node->prop_as_string(prop_choices);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->Append(wxString::FromUTF8(iter));

        if (node->prop_as_string(prop_selection_string).size())
        {
            widget->SetStringSelection(wxString::FromUTF8(node->prop_as_string(prop_selection_string)));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
                widget->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> ListBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxListBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_window_name).empty() && node->prop_as_string(prop_type) == "wxLB_SINGLE" &&
        node->prop_as_string(prop_style).empty() && node->prop_as_string(prop_window_style).empty())
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", 0, nullptr, ";

        auto& type = node->prop_as_string(prop_type);
        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
            code << "0";
        else
        {
            code << type;
            if (style.size())
            {
                code << '|' << style;
            }
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }

        if (node->prop_as_string(prop_window_name).size())
        {
            code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> ListBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_choices).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_choices));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->Append(" << GenerateQuotedString(iter) << ");";
        }

        if (node->prop_as_string(prop_selection_string).size())
        {
            code << "\n";
            code << node->get_node_name() << "->SetStringSelection("
                 << GenerateQuotedString(node->prop_as_string(prop_selection_string)) << ");";
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
            {
                code << "\n";
                code << node->get_node_name() << "->SetSelection(" << node->prop_as_string(prop_selection_int) << ");";
            }
        }
    }

    return code;
}

std::optional<ttlib::cstr> ListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool ListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listbox.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  CheckListBoxGenerator  //////////////////////////////////////////

wxObject* CheckListBoxGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxCheckListBox(
        wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos), node->prop_as_wxSize(prop_size), 0,
        nullptr, node->prop_as_int(prop_type) | node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    auto& items = node->prop_as_string(prop_choices);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->Append(wxString::FromUTF8(iter));

        if (node->prop_as_string(prop_selection_string).size())
        {
            widget->SetStringSelection(wxString::FromUTF8(node->prop_as_string(prop_selection_string)));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
                widget->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> CheckListBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxCheckListBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_window_name).empty() && node->prop_as_string(prop_type) == "wxLB_SINGLE" &&
        node->prop_as_string(prop_style).empty() && node->prop_as_string(prop_window_style).empty())
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", 0, nullptr, ";

        auto& type = node->prop_as_string(prop_type);
        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
            code << "0";
        else
        {
            code << type;
            if (style.size())
            {
                code << '|' << style;
            }
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }

        if (node->prop_as_string(prop_window_name).size())
        {
            code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> CheckListBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_choices).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_choices));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->Append(" << GenerateQuotedString(iter) << ");";
        }

        if (node->prop_as_string(prop_selection_string).size())
        {
            code << "\n";
            code << node->get_node_name() << "->SetStringSelection("
                 << GenerateQuotedString(node->prop_as_string(prop_selection_string)) << ");";
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
            {
                code << "\n";
                code << node->get_node_name() << "->SetSelection(" << node->prop_as_string(prop_selection_int) << ");";
            }
        }
    }

    return code;
}

std::optional<ttlib::cstr> CheckListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool CheckListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checklst.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  RearrangeCtrlGenerator  //////////////////////////////////////////

wxObject* RearrangeCtrlGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxRearrangeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                      node->prop_as_wxSize(prop_size), wxArrayInt(), wxArrayString(),
                                      node->prop_as_int(prop_type) | node->prop_as_int(prop_style) |
                                          node->prop_as_int(prop_window_style));

    auto& items = node->prop_as_string(prop_choices);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->GetList()->Append(wxString::FromUTF8(iter));

        if (node->prop_as_string(prop_selection_string).size())
        {
            widget->GetList()->SetStringSelection(wxString::FromUTF8(node->prop_as_string(prop_selection_string)));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
                widget->GetList()->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> RearrangeCtrlGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxRearrangeCtrl(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    code << ", ";
    GenPos(node, code);
    code << ", ";
    GenSize(node, code);
    code << ", wxArrayInt(), wxArrayString()";

    auto& type = node->prop_as_string(prop_type);
    auto& style = node->prop_as_string(prop_style);
    auto& win_style = node->prop_as_string(prop_window_style);

    if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
    {
        if (node->HasValue(prop_window_name))
        {
            code << ", 0";
        }
    }
    else
    {
        code << ", " << type;
        if (style.size())
        {
            code << '|' << style;
        }
        if (win_style.size())
        {
            code << '|' << win_style;
        }
    }

    if (node->prop_as_string(prop_window_name).size())
    {
        code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
    }
    code << ");";

    return code;
}

std::optional<ttlib::cstr> RearrangeCtrlGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_choices).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_choices));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->GetList()->Append(" << GenerateQuotedString(iter) << ");";
        }

        if (node->prop_as_string(prop_selection_string).size())
        {
            code << "\n";
            code << node->get_node_name() << "->GetList()->SetStringSelection("
                 << GenerateQuotedString(node->prop_as_string(prop_selection_string)) << ");";
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<int>(array.size()))
            {
                code << "\n";
                code << node->get_node_name() << "->GetList()->SetSelection(" << node->prop_as_string(prop_selection_int)
                     << ");";
            }
        }
    }

    return code;
}

std::optional<ttlib::cstr> RearrangeCtrlGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool RearrangeCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/rearrangectrl.h>", set_src, set_hdr);
    return true;
}

//////////////////////////////////////////  HtmlListBoxGenerator  //////////////////////////////////////////

wxObject* HtmlListBoxGenerator::Create(Node* node, wxObject* parent)
{
    auto widget = new wxSimpleHtmlListBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxPoint(prop_pos),
                                          node->prop_as_wxSize(prop_size), 0, nullptr,
                                          node->prop_as_int(prop_style) | node->prop_as_int(prop_window_style));

    auto& items = node->prop_as_string(prop_choices);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->Append(wxString::FromUTF8(iter));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << " = new wxSimpleHtmlListBox(";
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_window_name).empty() && node->prop_as_string(prop_type) == "wxLB_SINGLE" &&
        node->prop_as_string(prop_style).empty() && node->prop_as_string(prop_window_style).empty())
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", 0, nullptr, ";

        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (style.empty() && win_style.empty())
            code << "0";
        else
        {
            if (style.size())
            {
                code << style;
            }
            if (win_style.size())
            {
                if (style.size())
                    code << '|';
                code << win_style;
            }
        }

        if (node->prop_as_string(prop_window_name).size())
        {
            code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_string(prop_choices).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_choices));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->Append(" << GenerateQuotedString(iter) << ");";
        }
    }

    return code;
}

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HtmlListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/htmllbox.h>", set_src, set_hdr);
    return true;
}
