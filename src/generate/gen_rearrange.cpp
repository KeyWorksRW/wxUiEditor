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
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_rearrange.h"

wxObject* RearrangeCtrlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxRearrangeCtrl(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                      DlgSize(parent, node, prop_size), wxArrayInt(), wxArrayString(),
                                      node->prop_as_int(prop_type) | GetStyleInt(node));

    auto items = node->as_checklist_items(prop_contents);
    if (items.size())
    {
        for (auto& iter: items)
        {
            auto pos = widget->GetList()->Append(iter.label.wx_str());
            if (iter.checked.wx_str() == "1")
                widget->GetList()->Check(pos);
        }

        if (node->prop_as_string(prop_selection_string).size())
        {
            widget->GetList()->SetStringSelection(wxString::FromUTF8(node->prop_as_string(prop_selection_string)));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) widget->GetList()->GetCount())
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
        code << "auto* ";
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

std::optional<ttlib::cstr> RearrangeCtrlGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (node->HasValue(prop_contents))
    {
        auto contents = node->as_checklist_items(prop_contents);
        bool checked_item = false;
        for (auto& iter: contents)
        {
            if (iter.checked == "1")
            {
                checked_item = true;
                break;
            }
        }

        if (!checked_item)
        {
            for (auto& iter: contents)
            {
                if (code.size())
                    code << "\n";
                code << node->get_node_name() << "->GetList()->Append(" << GenerateQuotedString(iter.label) << ");";
            }
        }
        else
        {
            auto_indent = indent::auto_keep_whitespace;
            code << "{\n\t";
            code << "int item_position;";
            for (auto& iter: contents)
            {
                code << "\n\t";
                if (iter.checked == "1")
                    code << "item_position = ";
                code << node->get_node_name() << "->GetList()->Append(" << GenerateQuotedString(iter.label) << ");";
                if (iter.checked == "1")
                    code << "\n\t" << node->get_node_name() << "->GetList()->Check(item_position);";
            }
            code << "\n}";
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
            if (sel > -1 && sel < (to_int) contents.size())
            {
                code << "\n";
                code << node->get_node_name() << "->GetList()->SetSelection(" << node->value(prop_selection_int) << ");";
            }
        }
    }

    return code;
}

int RearrangeCtrlGenerator::GetRequiredVersion(Node* node)
{
    if (node->HasValue(prop_contents))
    {
        return minRequiredVer + 1;
    }

    return minRequiredVer;
}

bool RearrangeCtrlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/rearrangectrl.h>", set_src, set_hdr);
    return true;
}
