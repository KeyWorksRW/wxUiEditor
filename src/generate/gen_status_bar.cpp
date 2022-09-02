//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStatusBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/statusbr.h>  // wxStatusBar class interface

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_status_bar.h"

wxObject* StatusBarGenerator::CreateMockup(Node* node, wxObject* parent)
{
    auto org_style = GetStyleInt(node);
    // Don't display the gripper as it can resize our main window rather than just the mockup window
    auto widget = new wxStatusBar(wxStaticCast(parent, wxWindow), wxID_ANY, (org_style &= ~wxSTB_SIZEGRIP));

    auto fields = node->as_statusbar_fields(prop_fields);
    if (fields.size())
    {
        bool set_width = false;
        bool set_style = false;
        for (auto& iter: fields)
        {
            if (iter.width.size() && iter.width.atoi() != -1)
                set_width = true;
            if (iter.style.size() && iter.style != "wxSB_NORMAL")
                set_style = true;
        }

        if (set_width)
        {
            std::vector<int> widths;
            for (auto& iter: fields)
            {
                widths.push_back(iter.width.atoi());
            }
            widget->SetFieldsCount(static_cast<int>(widths.size()), widths.data());
        }
        else
        {
            widget->SetFieldsCount(to_int(fields.size()));
        }
        if (set_style)
        {
            std::vector<int> styles;
            for (auto& iter: fields)
            {
                styles.push_back(g_NodeCreator.GetConstantAsInt(iter.style));
            }
            widget->SetStatusStyles(to_int(styles.size()), styles.data());
        }
    }
    else
    {
        // The default is to have one field
        widget->SetFieldsCount(1);
    }

    if (org_style & wxSTB_SIZEGRIP)
        widget->SetStatusText("gripper not displayed in Mock Up");

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenConstruction(Node* node)
{
    ttlib::cstr code;

    int num_fields;
    auto fields = node->as_statusbar_fields(prop_fields);
    if (GetRequiredVersion(node) > minRequiredVer)
        num_fields = to_int(fields.size());
    else
        num_fields = node->prop_as_int(prop_fields);

    if (node->IsLocal())
        code << "auto* ";
    code << node->get_node_name() << " = CreateStatusBar(";

    if (node->prop_as_string(prop_window_name).size())
    {
        code << num_fields << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
        code << ", " << node->prop_as_string(prop_window_name);
    }
    else if (node->prop_as_int(prop_style) != wxSTB_DEFAULT_STYLE || node->prop_as_int(prop_window_style) > 0)
    {
        code << num_fields << ", " << node->prop_as_string(prop_id);
        GenStyle(node, code);
    }
    else if (node->prop_as_string(prop_id) != "wxID_ANY")
    {
        code << num_fields << ", " << node->prop_as_string(prop_id);
    }
    else if (num_fields > 1)
    {
        code << num_fields;
    }

    code << ");";

    return code;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenSettings(Node* node, size_t& auto_indent)
{
    if (GetRequiredVersion(node) <= minRequiredVer)
        return {};

    ttlib::cstr code;
    auto_indent = indent::auto_keep_whitespace;
    code << "{\n\t";

    auto fields = node->as_statusbar_fields(prop_fields);
    ttlib::cstr widths, styles;
    for (auto& iter: fields)
    {
        if (widths.size())
            widths << ", ";
        widths << iter.width;
        if (styles.size())
            styles << ", ";
        styles << iter.style;
    }
    code << "const int sb_field_widths[" << fields.size() << "] = {" << widths << "};\n\t";
    code << node->get_node_name() << "->SetStatusWidths(" << fields.size() << ", sb_field_widths);\n\t";
    code << "const int sb_field_styles[" << fields.size() << "] = {" << widths << "};\n\t";
    code << node->get_node_name() << "->SetStatusStyles(" << fields.size() << ", sb_field_styles);\n";
    code << "}";

    return code;
}

std::optional<ttlib::cstr> StatusBarGenerator::GenEvents(NodeEvent* event, const std::string& class_name)
{
    return GenEventCode(event, class_name);
}

int StatusBarGenerator::GetRequiredVersion(Node* node)
{
    if (!node->HasValue(prop_fields))
        return minRequiredVer;
    if (ttlib::is_digit(node->value(prop_fields)[0]))
        return minRequiredVer;
    return minRequiredVer + 1;
}

bool StatusBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/statusbr.h>", set_src, set_hdr);
    return true;
}

int StatusBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->GetParent()->IsSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStatusBar");

    if (GetRequiredVersion(node) > minRequiredVer)
    {
        auto fields = node->as_statusbar_fields(prop_fields);
        if (fields.size())
        {
            ttlib::cstr widths, styles;
            for (auto& iter: fields)
            {
                if (widths.size())
                    widths << ",";
                widths << iter.width;
                if (styles.size())
                    styles << ",";
                styles << iter.style;
            }
            item.append_child("fields").text().set(ttlib::cstr() << fields.size());
            item.append_child("widths").text().set(widths);
            item.append_child("styles").text().set(styles);
        }
    }
    else
    {
        ADD_ITEM_PROP(prop_fields, "fields")
    }

    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    if (xrc_flags & xrc::add_comments)
    {
        GenXrcComments(node, item);
    }

    return result;
}

void StatusBarGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxStatusBarXmlHandler");
}
