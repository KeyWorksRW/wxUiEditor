/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCheckListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/checklst.h>  // wxCheckListBox class interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_check_listbox.h"

using namespace code;

wxObject* CheckListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxCheckListBox(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                           DlgSize(parent, node, prop_size), 0, nullptr, node->prop_as_int(prop_type) | GetStyleInt(node));

    auto items = node->as_checklist_items(prop_contents);
    if (items.size())
    {
        for (auto& iter: items)
        {
            auto pos = widget->Append(iter.label.make_wxString());
            if (iter.checked == "1")
                widget->Check(pos);
        }

        if (node->as_string(prop_selection_string).size())
        {
            widget->SetStringSelection(node->as_wxString(prop_selection_string));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) widget->GetCount())
                widget->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool CheckListBoxGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    auto params_needed = code.WhatParamsNeeded();
    if (params_needed != nothing_needed || !code.IsEqualTo(prop_type, "wxLB_SINGLE"))
    {
        code.Comma().Pos().Comma().WxSize();
        code.Comma();
        if (code.is_cpp())
            code += "0, nullptr";
        else
            code += "[]";
        code.Comma().Style(nullptr, code.node()->as_string(prop_type));

        if (params_needed & window_name_needed)
        {
            code.Comma().Add("wxDefaultValidator").QuotedString(prop_window_name);
        }
    }
    code.EndFunction();

    return true;
}

bool CheckListBoxGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
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
                code.Eol(eol_if_empty).NodeName().Function("Append(").QuotedString(iter.label).EndFunction();
            }
        }
        else
        {
            code.OpenBrace();
            if (code.is_cpp())
                code += "int item_position;";
            for (auto& iter: contents)
            {
                code.Eol(eol_if_empty);
                if (iter.checked == "1")
                    code += "item_position = ";
                code.NodeName().Function("Append(").QuotedString(iter.label).EndFunction();
                if (iter.checked == "1")
                    code.Eol().NodeName().Function("Check(item_position").EndFunction();
            }
            code.CloseBrace();
        }

        if (code.HasValue(prop_selection_string))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetStringSelection(");
            code.QuotedString(prop_selection_string).EndFunction();
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) contents.size())
            {
                code.Eol(eol_if_empty).NodeName().Function("SetSelection(").Str(prop_selection_int).EndFunction();
            }
        }
    }
    return true;
}

int CheckListBoxGenerator::GetRequiredVersion(Node* node)
{
    if (node->HasValue(prop_contents))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }

    return BaseGenerator::GetRequiredVersion(node);
}

bool CheckListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/checklst.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_chckl.cpp
// ../../../wxWidgets/src/xrc/xh_chckl.cpp

int CheckListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxCheckListBox");

    if (node->HasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = node->as_checklist_items(prop_contents);
        for (auto& iter: array)
        {
            auto child = content.append_child("item");
            child.text().set(iter.label);
            if (iter.checked == "1")
            {
                child.append_attribute("checked").set_value("1");
            }
        }
    }

    // TODO: [KeyWorks - 06-04-2022] This needs to be supported in XRC
    if (node->HasValue(prop_selection_string))
        item.append_child("value").text().set(node->as_string(prop_selection_string));

    // Older versions of wxWidgets didn't support setting the selection via the value property,
    // so we add the property here even if the above is set.
    if (node->prop_as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->as_string(prop_selection_int));

    GenXrcStylePosSize(node, item, prop_type);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void CheckListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxCheckListBoxXmlHandler");
}
