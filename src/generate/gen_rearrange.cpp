/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRearrangeCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/rearrangectrl.h>  // various controls for rearranging the items interactively

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_rearrange.h"

wxObject* RearrangeCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRearrangeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), wxArrayInt(), wxArrayString(),
                                      node->prop_as_int(prop_type) | GetStyleInt(node));

    auto& items = node->prop_as_string(prop_contents);
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
            if (sel > -1 && sel < (to_int) array.size())
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
    code << node->get_node_name() << GenerateNewAssignment(node);
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

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (node->prop_as_string(prop_contents).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
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
            if (sel > -1 && sel < (to_int) array.size())
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
