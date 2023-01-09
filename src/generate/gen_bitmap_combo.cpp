/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapComboBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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
    auto widget = new wxBitmapComboBox(wxStaticCast(parent, wxWindow), wxID_ANY, node->prop_as_wxString(prop_value),
                                       DlgPoint(parent, node, prop_pos), DlgSize(parent, node, prop_size), 0, nullptr,
                                       GetStyleInt(node));

    if (node->HasValue(prop_hint))
        widget->SetHint(node->prop_as_wxString(prop_hint));

    if (node->HasValue(prop_contents))
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
            widget->Append(iter.wx_str());

        if (node->HasValue(prop_selection_string))
        {
            widget->SetStringSelection(node->prop_as_wxString(prop_selection_string));
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) array.size())
                widget->SetSelection(sel);
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BitmapComboBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (!node->HasValue(prop_contents))
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
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.AddAuto().NodeName().CreateClass();
    code.ValidParentName().Comma().as_string(prop_id);
    if (code.HasValue(prop_style))
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
    if (code.HasValue(prop_hint) && !code.PropContains(prop_style, "wxCB_READONLY"))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetFocus(").EndFunction();
    }

    if (code.HasValue(prop_contents))
    {
        auto array = ConvertToArrayString(code.node()->as_string(prop_contents));
        for (auto& iter: array)
        {
            code.Eol(eol_if_empty).NodeName().Function("Append(").QuotedString(iter).EndFunction();
        }

        if (code.HasValue(prop_selection_string))
        {
            code.Eol(eol_if_empty);
            if (code.HasValue(prop_validator_variable))
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

bool BitmapComboBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/bmpcbox.h>", set_src, set_hdr);
    if (node->prop_as_string(prop_validator_variable).size())
        InsertGeneratorInclude(node, "#include <wx/valgen.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_bmpcbox.cpp
// ../../../wxWidgets/src/xrc/xh_bmpcbox.cpp

int BitmapComboBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxBitmapComboBox");

    if (node->HasValue(prop_selection_string))
        item.append_child("value").text().set(node->prop_as_string(prop_selection_string));
    else if (node->prop_as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->prop_as_string(prop_selection_int));

    if (node->HasValue(prop_hint) && !node->prop_as_string(prop_style).contains("wxCB_READONLY"))
        item.append_child("hint").text().set(node->prop_as_string(prop_hint));

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
