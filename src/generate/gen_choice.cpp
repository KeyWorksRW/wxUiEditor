//////////////////////////////////////////////////////////////////////////
// Purpose:   wxChoice generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/choice.h>  // wxChoice class interface

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_choice.h"

wxObject* ChoiceGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxChoice(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
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

bool ChoiceGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (!node->hasValue(prop_contents))
        return false;

    if (prop->isProp(prop_selection_string))
    {
        wxStaticCast(widget, wxChoice)->SetStringSelection(prop->as_wxString());
        return true;
    }
    else if (prop->isProp(prop_selection_int))
    {
        wxStaticCast(widget, wxChoice)->SetSelection(prop->as_int());
        return true;
    }
    return false;
}

bool ChoiceGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    if (code.hasValue(prop_style))
    {
        code.Comma().Pos().Comma().CheckLineLength().WxSize();
        if (code.is_cpp())
        {
            code.Comma().CheckLineLength(sizeof("0, nullptr, ") + code.node()->as_string(prop_style).size());
            code << "0, nullptr";
        }
        else
        {
            code.Comma().CheckLineLength(sizeof("[], ") + code.node()->as_string(prop_style).size());
            code.Add("[]");
        }
        code.Comma().Style().EndFunction();
    }
    else
    {
        code.PosSizeFlags(true);
    }

    return true;
}

bool ChoiceGenerator::SettingsCode(Code& code)
{
    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetFocus(").EndFunction();
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

bool ChoiceGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/choice.h>", set_src, set_hdr);
    return true;
}

int ChoiceGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxChoice");

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

    // Older versions of wxWidgets didn't support setting the selection via the value property,
    // so we add the property here even if the above is set.
    if (node->as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->as_string(prop_selection_int));

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

void ChoiceGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxChoiceXmlHandler");
}
