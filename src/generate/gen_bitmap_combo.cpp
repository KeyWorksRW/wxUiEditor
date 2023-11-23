/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapComboBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bmpcbox.h>  // wxBitmapComboBox base header

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_bitmap_combo.h"

wxObject* BitmapComboBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget = new wxBitmapComboBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->as_wxString(prop_value),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), 0, nullptr,
                                       GetStyleInt(node));

    if (node->hasValue(prop_hint))
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

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BitmapComboBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (!node->hasValue(prop_contents))
        return false;

    if (prop->isProp(prop_selection_string))
    {
        wxStaticCast(widget, wxBitmapComboBox)->SetStringSelection(prop->as_wxString());
        return true;
    }
    else if (prop->isProp(prop_selection_int))
    {
        wxStaticCast(widget, wxBitmapComboBox)->SetSelection(prop->as_int());
        return true;
    }
    return false;
}

bool BitmapComboBoxGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    if (code.hasValue(prop_style))
    {
        code.Comma().Add("wxEmptyString");
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
        if (code.WhatParamsNeeded() != nothing_needed)
        {
            code.Comma().Add("wxEmptyString");
            code.PosSizeFlags(true);
        }
        else
        {
            code.EndFunction();
        }
    }

    return true;
}

bool BitmapComboBoxGenerator::SettingsCode(Code& code)
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

bool BitmapComboBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                          int /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/bmpcbox.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
        set_src.insert("#include <wx/valgen.h>");
    return true;
}

// ../../wxSnapShot/src/xrc/xh_bmpcbox.cpp
// ../../../wxWidgets/src/xrc/xh_bmpcbox.cpp

int BitmapComboBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxBitmapComboBox");

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
        GenXrcComments(node, item);
    }

    return result;
}

void BitmapComboBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxBitmapComboBoxXmlHandler");
}
