/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/listbox.h>  // wxListBox class interface

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class
#include "utils.h"       // Utility functions that work with properties

#include "gen_listbox.h"

using namespace code;

wxObject* ListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxListBox(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                DlgSize(parent, node, prop_size), 0, nullptr, node->as_int(prop_type) | GetStyleInt(node));

    if (node->hasValue(prop_contents))
    {
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
            widget->Append(iter.make_wxString());

        if (node->as_string(prop_selection_string).size())
        {
            widget->SetStringSelection(node->as_wxString(prop_selection_string));
        }
        else
        {
            int sel = node->as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) array.size())
                widget->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ListBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
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

bool ListBoxGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_empty).NodeName().Function("SetFocus(").EndFunction();
    }

    if (code.hasValue(prop_contents))
    {
        auto array = code.node()->as_ArrayString(prop_contents);
        for (auto& iter: array)
        {
            code.Eol(eol_if_empty).NodeName().Function("Append(").QuotedString(iter).EndFunction();
        }

        if (code.hasValue(prop_selection_string))
        {
            code.Eol(eol_if_empty).NodeName().Function("SetStringSelection(");
            code.QuotedString(prop_selection_string).EndFunction();
        }
        else
        {
            int sel = code.IntValue(prop_selection_int);
            if (sel > -1 && sel < (to_int) array.size())
            {
                code.Eol(eol_if_empty).NodeName().Function("SetSelection(").as_string(prop_selection_int).EndFunction();
            }
        }
    }
    return true;
}

bool ListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/listbox.h>", set_src, set_hdr);
    if (node->hasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_listb.cpp
// ../../../wxWidgets/src/xrc/xh_listb.cpp

int ListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxListBox");

    if (node->hasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    // TODO: [KeyWorks - 06-04-2022] This needs to be supported in XRC
    if (node->hasValue(prop_selection_string))
        item.append_child("value").text().set(node->as_string(prop_selection_string));

    // Older versions of wxWidgets didn't support setting the selection via the value property,
    // so we add the property here even if the above is set.
    if (node->as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->as_string(prop_selection_int));

    GenXrcStylePosSize(node, item, prop_type);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void ListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxListBoxXmlHandler");
}
