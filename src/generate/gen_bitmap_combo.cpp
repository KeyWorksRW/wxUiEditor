/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapComboBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/bmpcbox.h>  // wxBitmapComboBox base header

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "image_handler.h"  // ImageHandler class
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties

#include "gen_bitmap_combo.h"

// Converts the filename stored in a bmp_combo_item into a bundle description string.
// Returns an empty string when the item has no bitmap.
static wxue::string get_ItemBitmapDescription(const wxue::string& bitmap)
{
    wxue::string description;
    if (bitmap.empty())
    {
        return description;
    }

    if (bitmap.extension().is_sameas(".svg", wxue::CASE::either))
    {
        description << "SVG;" << bitmap << ";[16,16]";
    }
    else
    {
        description << "Embed;" << bitmap;
    }

    return description;
}

wxObject* BitmapComboBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto* widget = new wxBitmapComboBox(wxStaticCast(parent, wxWindow), wxID_ANY,
                                        node->as_wxString(prop_value), DlgPoint(node, prop_pos),
                                        DlgSize(node, prop_size), 0, nullptr, GetStyleInt(node));

    if (node->HasValue(prop_hint))
    {
        widget->SetHint(node->as_wxString(prop_hint));
    }

    if (node->HasValue(prop_contents))
    {
        const std::vector<NODEPROP_BMP_COMBO_ITEM> items = node->as_bmp_combo_items(prop_contents);
        for (const auto& item: items)
        {
            wxBitmapBundle bundle;
            if (!item.bitmap.empty())
            {
                bundle = ProjectImages.GetBitmapBundle(get_ItemBitmapDescription(item.bitmap));
            }
            widget->Append(item.label.wx(), bundle);
        }

        if (node->HasValue(prop_selection_string))
        {
            widget->SetStringSelection(node->as_wxString(prop_selection_string));
        }
        else
        {
            int sel = node->as_int(prop_selection_int);
            if (sel > -1 && sel < static_cast<to_int>(items.size()))
            {
                widget->SetSelection(sel);
            }
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

bool BitmapComboBoxGenerator::OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop)
{
    if (!node->HasValue(prop_contents))
    {
        return false;
    }

    if (prop->isProp(prop_selection_string))
    {
        wxStaticCast(widget, wxBitmapComboBox)->SetStringSelection(prop->as_wxString());
        return true;
    }
    if (prop->isProp(prop_selection_int))
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
    if (code.HasValue(prop_style))
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

bool BitmapComboBoxGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_hint) && !code.PropContains(prop_style, "wxCB_READONLY"))
    {
        code.Eol(eol_if_empty);
        code.NodeName().Function("SetHint(").QuotedString(prop_hint).EndFunction();
    }

    if (code.IsTrue(prop_focus))
    {
        auto* form = code.node()->get_Form();
        // wxDialog and wxFrame will set the focus to this control after all controls are created.
        if (!form->is_Gen(gen_wxDialog) && !form->is_Type(type_frame_form))
        {
            code.Eol(eol_if_empty);
            code.NodeName().Function("SetFocus(").EndFunction();
        }
    }

    if (code.HasValue(prop_contents))
    {
        const std::vector<NODEPROP_BMP_COMBO_ITEM> items =
            code.node()->as_bmp_combo_items(prop_contents);
        for (const auto& item: items)
        {
            if (item.bitmap.empty())
            {
                code.Eol(eol_if_empty)
                    .NodeName()
                    .Function("Append(")
                    .QuotedString(item.label)
                    .EndFunction();
            }
            else if (code.is_cpp())
            {
                wxue::string bundle_code;
                const bool is_vector_code =
                    GenerateBundleCode(get_ItemBitmapDescription(item.bitmap), bundle_code);
                if (is_vector_code)
                {
                    code.Eol(eol_if_empty);
                    code += bundle_code;
                    code.Tab().NodeName().Function("Append(").QuotedString(item.label).Comma();
                    code += "wxBitmapBundle::FromBitmaps(bitmaps)";
                    code.EndFunction().CloseBrace();
                }
                else
                {
                    code.Eol(eol_if_empty)
                        .NodeName()
                        .Function("Append(")
                        .QuotedString(item.label)
                        .Comma();
                    code += bundle_code;
                    code.EndFunction();
                }
            }
            else
            {
                code.Eol(eol_if_empty)
                    .NodeName()
                    .Function("Append(")
                    .QuotedString(item.label)
                    .Comma();
                code.Bundle(get_ItemBitmapDescription(item.bitmap));
                code.EndFunction();
            }
        }

        if (code.HasValue(prop_selection_string))
        {
            code.Eol(eol_if_empty);
            if (code.HasValue(prop_validator_variable))
            {
                code.as_string(prop_validator_variable) << " = ";
                code.QuotedString(prop_selection_string);
                if (code.is_cpp())
                {
                    code << ";  // set validator variable";
                }
                else
                {
                    code << "  # set validator variable";
                }
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
            if (sel > -1 && sel < static_cast<to_int>(items.size()))
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

bool BitmapComboBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                          std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/bmpcbox.h>", set_src, set_hdr);
    if (node->as_string(prop_validator_variable).size())
    {
        set_src.insert("#include <wx/valgen.h>");
    }
    return true;
}

// ../../wxSnapShot/src/xrc/xh_bmpcbox.cpp
// ../../../wxWidgets/src/xrc/xh_bmpcbox.cpp

int BitmapComboBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxBitmapComboBox");

    if (node->HasValue(prop_selection_string))
    {
        item.append_child("value").text().set(node->as_string(prop_selection_string));
    }
    else if (node->as_int(prop_selection_int) >= 0)
    {
        item.append_child("selection").text().set(node->as_string(prop_selection_int));
    }

    if (node->HasValue(prop_hint) && !node->as_string(prop_style).contains("wxCB_READONLY"))
    {
        item.append_child("hint").text().set(node->as_string(prop_hint));
    }

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
