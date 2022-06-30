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

std::optional<ttlib::cstr> BitmapComboBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id) << ", ";

    // We don't add any strings until after the constructor, so if an initial selection string is specified, we set it
    // after all strings have been appended.
    code << "wxEmptyString";

    if (node->prop_as_string(prop_window_name).empty() && node->prop_as_string(prop_style).empty() &&
        node->prop_as_string(prop_window_style).empty())
    {
        GeneratePosSizeFlags(node, code);
    }
    else
    {
        // We have to generate a default validator before the window name, which GeneratePosSizeFlags doesn't do. We don't
        // actually need that validator, since GenSettings will create it, but we have to supply something before the window
        // name.

        code << ", ";
        GenPos(node, code);
        code << ", ";
        GenSize(node, code);
        code << ", 0, nullptr, ";
        GenStyle(node, code);
        if (node->HasValue(prop_window_name))
        {
            code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    code.Replace(", wxID_ANY, wxEmptyString);", ");");

    return code;
}

std::optional<ttlib::cstr> BitmapComboBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->HasValue(prop_hint))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetHint(" << GenerateQuotedString(node->prop_as_string(prop_hint)) << ");";
    }

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (node->HasValue(prop_contents))
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->Append(" << GenerateQuotedString(iter) << ");";
        }

        if (node->HasValue(prop_selection_string))
        {
            code << "\n";
            if (node->HasValue(prop_validator_variable))
            {
                code << node->prop_as_string(prop_validator_variable) << " = ";
                code << GenerateQuotedString(node, prop_selection_string) << ";  // set validator variable";
            }
            else
            {
                code << node->get_node_name() << "->SetStringSelection(";
                code << GenerateQuotedString(node->prop_as_string(prop_selection_string)) << ");";
            }
        }
        else
        {
            int sel = node->prop_as_int(prop_selection_int);
            if (sel > -1 && sel < (to_int) array.size())
            {
                code << "\n";
                code << node->get_node_name() << "->SetSelection(" << node->prop_as_string(prop_selection_int) << ");";
            }
        }
    }

    return code;
}

std::optional<ttlib::cstr> BitmapComboBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool BitmapComboBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/bmpcbox.h>", set_src, set_hdr);
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
