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

std::optional<ttlib::sview> RearrangeCtrlGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.GetParentName().Comma().as_string(prop_id);
    code.Comma().Pos().Comma().WxSize();
    code.Comma();
    if (code.is_cpp())
        code += "wxArrayInt(), wxArrayString()";
    else
        code += "[], []";

    Node* node = code.node();
    auto& type = node->prop_as_string(prop_type);
    auto& style = node->prop_as_string(prop_style);
    auto& win_style = node->prop_as_string(prop_window_style);

    if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
    {
        if (node->HasValue(prop_window_name))
        {
            code += ", 0";
        }
    }
    else
    {
        code.Comma().Add(type).Comma().Style();
    }

    if (node->HasValue(prop_window_name))
    {
        code.Comma().Add("wxDefaultValidator").Comma().QuotedString(prop_window_name);
    }
    code.EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> RearrangeCtrlGenerator::CommonSettings(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.Eol(true).NodeName().Function("SetFocus(").EndFunction();
    }
    if (code.HasValue(prop_contents))
    {
        Node* node = code.node();
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
                code.Eol(true).NodeName().Function("GetList()").Function("Append(").QuotedString(iter.label).EndFunction();
            }
        }
        else
        {
            code.OpenBrace();
            if (code.is_cpp())
                code += "int item_position;";
            for (auto& iter: contents)
            {
                code.Eol(true);
                if (iter.checked == "1")
                    code += "item_position = ";
                code.NodeName().Function("GetList()").Function("Append(").QuotedString(iter.label).EndFunction();
                if (iter.checked == "1")
                    code.Eol().NodeName().Function("GetList()").Function("Check(item_position").EndFunction();
            }
            code.CloseBrace();
        }

        if (code.HasValue(prop_selection_string))
        {
            code.Eol(true).NodeName().Function("GetList()").Function("SetStringSelection(");
            code.QuotedString(prop_selection_string).EndFunction();
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) contents.size())
            {
                code.Eol(true)
                    .NodeName()
                    .Function("GetList()")
                    .Function("SetSelection(")
                    .Str(prop_selection_int)
                    .EndFunction();
            }
        }
    }
    return code.m_code;
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
