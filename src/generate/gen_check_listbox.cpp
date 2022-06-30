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

#include "gen_check_listbox.h"

//////////////////////////////////////////  CheckListBoxGenerator  //////////////////////////////////////////

wxObject* CheckListBoxGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto widget =
        new wxCheckListBox(wxStaticCast(parent, wxWindow), wxID_ANY, DlgPoint(parent, node, prop_pos),
                           DlgSize(parent, node, prop_size), 0, nullptr, node->prop_as_int(prop_type) | GetStyleInt(node));

    auto& items = node->prop_as_string(prop_contents);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->Append(wxString::FromUTF8(iter));

        if (node->prop_as_string(prop_selection_string).size())
        {
            widget->SetStringSelection(wxString::FromUTF8(node->prop_as_string(prop_selection_string)));
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

std::optional<ttlib::cstr> CheckListBoxGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;
    if (node->IsLocal())
        code << "auto ";
    code << node->get_node_name() << GenerateNewAssignment(node);
    code << GetParentName(node) << ", " << node->prop_as_string(prop_id);

    if (node->prop_as_string(prop_window_name).empty() && node->prop_as_string(prop_type) == "wxLB_SINGLE" &&
        node->prop_as_string(prop_style).empty() && node->prop_as_string(prop_window_style).empty())
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

        auto& type = node->prop_as_string(prop_type);
        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (type == "wxLB_SINGLE" && style.empty() && win_style.empty())
            code << "0";
        else
        {
            code << type;
            if (style.size())
            {
                code << '|' << style;
            }
            if (win_style.size())
            {
                code << '|' << win_style;
            }
        }

        if (node->prop_as_string(prop_window_name).size())
        {
            code << ", wxDefaultValidator, " << node->prop_as_string(prop_window_name);
        }
        code << ");";
    }

    return code;
}

std::optional<ttlib::cstr> CheckListBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
{
    ttlib::cstr code;

    if (node->prop_as_bool(prop_focus))
    {
        if (code.size())
            code << '\n';
        code << node->get_node_name() << "->SetFocus()";
    }

    if (node->prop_as_string(prop_contents).size())
    {
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
        {
            if (code.size())
                code << "\n";
            code << node->get_node_name() << "->Append(" << GenerateQuotedString(iter) << ");";
        }

        if (node->prop_as_string(prop_selection_string).size())
        {
            code << "\n";
            code << node->get_node_name() << "->SetStringSelection("
                 << GenerateQuotedString(node->prop_as_string(prop_selection_string)) << ");";
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

std::optional<ttlib::cstr> CheckListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
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
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    // TODO: [KeyWorks - 06-04-2022] This needs to be supported in XRC
    if (node->HasValue(prop_selection_string))
        item.append_child("value").text().set(node->prop_as_string(prop_selection_string));

    // Older versions of wxWidgets didn't support setting the selection via the value property,
    // so we add the property here even if the above is set.
    if (node->prop_as_int(prop_selection_int) >= 0)
        item.append_child("selection").text().set(node->prop_as_string(prop_selection_int));

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
