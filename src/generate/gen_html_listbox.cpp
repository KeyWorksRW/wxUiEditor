/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxSimpleHtmlListBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
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

    auto& items = node->prop_as_string(prop_contents);
    if (items.size())
    {
        auto array = ConvertToArrayString(items);
        for (auto& iter: array)
            widget->Append(wxString::FromUTF8(iter));
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenConstruction(Node* node)
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

        auto& style = node->prop_as_string(prop_style);
        auto& win_style = node->prop_as_string(prop_window_style);

        if (style.empty() && win_style.empty())
            code << "0";
        else
        {
            if (style.size())
            {
                code << style;
            }
            if (win_style.size())
            {
                if (style.size())
                    code << '|';
                code << win_style;
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

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenSettings(Node* node, size_t& /* auto_indent */)
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
    }

    return code;
}

std::optional<ttlib::cstr> HtmlListBoxGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

bool HtmlListBoxGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/htmllbox.h>", set_src, set_hdr);
    return true;
}

// ../../wxSnapShot/src/xrc/xh_simplehtmllbox.cpp
// ../../../wxWidgets/src/xrc/xh_simplehtmllbox.cpp

int HtmlListBoxGenerator::GenXrcObject(Node* node, pugi::xml_node& object, bool add_comments)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxSimpleHtmlListBox");

    if (node->HasValue(prop_contents))
    {
        auto content = item.append_child("content");
        auto array = ConvertToArrayString(node->prop_as_string(prop_contents));
        for (auto& iter: array)
        {
            content.append_child("item").text().set(iter);
        }
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void HtmlListBoxGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxSimpleHtmlListBoxXmlHandler");
}
