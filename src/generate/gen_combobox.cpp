/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxComboBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/combobox.h>  // wxComboBox declaration

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "utils.h"          // Utility functions that work with properties

#include "gen_combobox.h"

using namespace code;

wxObject* ComboBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxComboBox(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                 DlgPoint(node, prop_pos), DlgSize(node, prop_size), 0, nullptr,
                                 GetStyleInt(node));

    if (node->hasValue(prop_hint) && !node->as_string(prop_style).contains("wxCB_READONLY"))
        widget->SetHint(node->as_wxString(prop_hint));

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

    // Note that this event only gets fired of the drop-down button is clicked. Clicking in the edit
    // area does not generate an event (or at least it doesn't on Windows 10).

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool ComboBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    if (code.hasValue(prop_style))
    {
        code.Comma().Add("wxEmptyString");
        code.Comma().Pos().Comma().CheckLineLength().WxSize();
        if (code.is_cpp())
        {
            code.Comma().CheckLineLength(sizeof("0, nullptr, ") +
                                         code.node()->as_string(prop_style).size());
            code << "0, nullptr";
        }
        else
        {
            code.Comma().CheckLineLength(sizeof("[], ") +
                                         code.node()->as_string(prop_style).size());
            code.Add("[]");
        }
        code.Comma().Style().EndFunction();
    }
    else
    {
        if (code.WhatParamsNeeded() != nothing_needed)
        {
            code.Comma().Add("wxEmptyString");
            code.PosSizeFlags(code::allow_scaling, true);
        }
        else
        {
            code.EndFunction();
        }
    }

    return true;
}

bool ComboBoxGenerator::SettingsCode(Code& code)
{
    if (code.hasValue(prop_hint) && !code.PropContains(prop_style, "wxCB_READONLY"))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        auto form = code.node()->getForm();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->isGen(gen_wxDialog) && !form->isType(type_frame_form))
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
                code.Eol(eol_if_empty)
                    .NodeName()
                    .Function("SetSelection(")
                    .as_string(prop_selection_int)
                    .EndFunction();
            }
        }
    }

    return true;
}

bool ComboBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                    std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/combobox.h>", set_src, set_hdr);
    if (node->hasValue(prop_validator_variable))
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

int ComboBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created :
                                                 BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxComboBox");

    if (node->hasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = node->as_ArrayString(prop_contents);
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    if (node->hasValue(prop_selection_string))
        item.append_child("value").text().set(node->as_string(prop_selection_string));
    else if (node->as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->as_string(prop_selection_int));

    if (node->hasValue(prop_hint) && !node->as_string(prop_style).contains("wxCB_READONLY"))
        item.append_child("hint").text().set(node->as_string(prop_hint));

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

void ComboBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxComboBoxXmlHandler");
}

bool ComboBoxGenerator::GetImports(Node* /* node */, std::set<std::string>& set_imports,
                                   GenLang language)
{
    if (language == GEN_LANG_PERL)
    {
        set_imports.emplace("use Wx qw[:combobox];");
    }

    return false;
}
