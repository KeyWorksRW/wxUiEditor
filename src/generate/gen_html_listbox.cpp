/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSimpleHtmlListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/htmllbox.h>  // wxHtmlListBox is a listbox whose items are wxHtmlCells

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_html_listbox.h"

//////////////////////////////////////////  HtmlListBoxGenerator  //////////////////////////////////////////

wxObject* HtmlListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxSimpleHtmlListBox(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                                          DlgSize(parent, node, prop_size), 0, nullptr, GetStyleInt(node));

    if (node->hasValue(prop_contents))
    {
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
            widget->Append(iter.make_wxString());

        if (node->hasValue(prop_selection_string))
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

bool HtmlListBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().ValidParentName().Comma().as_string(prop_id);
    if (auto params_needed = code.WhatParamsNeeded("wxHLB_DEFAULT_STYLE"); params_needed != nothing_needed)
    {
        code.Comma().Pos().Comma().WxSize();
        if (code.is_cpp())
        {
            code.Comma().CheckLineLength(sizeof("0, nullptr, ") + code.node()->as_string(prop_style).size());
            code += "0, nullptr";
        }
        else
        {
            code.Comma().CheckLineLength(sizeof("[], ") + code.node()->as_string(prop_style).size());
            code.Add("[]");
        }
        code.Comma().Style();
        if (params_needed & window_name_needed)
        {
            code.Comma().Add("wxDefaultValidator").Comma().QuotedString(prop_window_name);
        }
    }

    code.EndFunction();
    return true;
}

bool HtmlListBoxGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isGen(gen_wxFrame))
        {
            code.Eol(eol_if_empty);
            code.NodeName().Function("SetFocus(").EndFunction();
        }
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
            code.Eol(eol_if_empty);
            if (code.hasValue(prop_validator_variable))
            {
                code.as_string(prop_validator_variable) << " = ";
                code.QuotedString(prop_selection_string);
                if (code.is_cpp())
                    code << ";  // set validator variable";
                else
                    code << "  # set validator variable";
            }
            else
            {
                code.NodeName().Function("SetStringSelection(");
                code.QuotedString(prop_selection_string).EndFunction();
            }
        }
        else
        {
            int sel = code.node()->as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) array.size())
            {
                code.Eol(eol_if_empty).NodeName().Function("SetSelection(").as_string(prop_selection_int).EndFunction();
            }
        }
    }

    return true;
}

bool HtmlListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/htmllbox.h>", set_src, set_hdr);
    return true;
}

bool HtmlListBoxGenerator::GetRubyImports(Node*, std::set<std::string>& set_imports)
{
    set_imports.insert("require 'wx/html'");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_simplehtmllbox.cpp
// ../../../wxWidgets/src/xrc/xh_simplehtmllbox.cpp

int HtmlListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSimpleHtmlListBox");

    if (node->hasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        if (node->hasValue(prop_selection_string))
        {
            ADD_ITEM_COMMENT("You cannot use selection_string for the selection in XRC.")
        }
        GenXrcComments(node, item);
    }

    return result;
}

void HtmlListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSimpleHtmlListBoxXmlHandler");
}
