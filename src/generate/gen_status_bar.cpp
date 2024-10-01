//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStatusBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/statusbr.h>  // wxStatusBar class interface

#include "code.h"           // Code -- Helper class for generating code
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
                styles.push_back(NodeCreation.getConstantAsInt(iter.style));
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

bool StatusBarGenerator::ConstructionCode(Code& code)
{
    Node* node = code.node();  // This is just for convenience
    int num_fields;
    auto fields = node->as_statusbar_fields(prop_fields);

    // GetRequiredVersion() checks see if the value starts with a digit -- if so, it's the
    // old style. If it isn't a digit, then it's a style which returns minRequiredVer+1.
    if (GetRequiredVersion(node) > minRequiredVer)
        num_fields = to_int(fields.size());
    else
        num_fields = node->as_int(prop_fields);

    if (node->hasValue(prop_subclass))
    {
        code.AddAuto().NodeName();
        code.CreateClass(false, "", true);
        code.ValidParentName().Comma().as_string(prop_id).Comma().Style();
        if (node->hasValue(prop_window_name))
        {
            code.Comma().QuotedString(prop_window_name);
        }
        code.EndFunction();
        if (num_fields > 0)
        {
            code.Eol().NodeName().Function("SetFieldsCount(").itoa(num_fields).EndFunction();
        }
        code.Eol().FormFunction("SetStatusBar(").NodeName().EndFunction();
        return true;
    }
    else
    {
        code.AddAuto().NodeName().Str(" = ").FormFunction("CreateStatusBar(");
    }

    if (node->hasValue(prop_window_name))
    {
        code.itoa(num_fields).Comma().as_string(prop_id).Comma().Style();
        code.Comma().QuotedString(prop_window_name);
    }
    else if (node->as_int(prop_style) != wxSTB_DEFAULT_STYLE || node->as_int(prop_window_style) > 0)
    {
        code.itoa(num_fields).Comma().as_string(prop_id).Comma().Style();
    }
    else if (node->as_string(prop_id) != "wxID_ANY")
    {
        code.itoa(num_fields).Comma().as_string(prop_id);
    }
    else if (num_fields > 1)
    {
        code.itoa(num_fields);
    }

    code.EndFunction();

    return true;
}

bool StatusBarGenerator::SettingsCode(Code& code)
{
    // A single field can be represeted by 1 which uses the older style of setting.
    if (GetRequiredVersion(code.node()) <= minRequiredVer)
        return true;

    auto fields = code.node()->as_statusbar_fields(prop_fields);
    tt_string widths, styles;
    for (auto& iter: fields)
    {
        if (widths.size())
            widths << ", ";
        widths << iter.width;
        if (styles.size())
            styles << ", ";
        styles << iter.style;
    }

    if (code.is_cpp())
    {
        code.OpenBrace();
        code << "const int sb_field_widths[" << fields.size() << "] = {" << widths << "};";
        code.Eol().NodeName().Function("SetStatusWidths(").itoa(fields.size()).Comma().Str("sb_field_widths);");
        code.Eol().Str("const int sb_field_styles[").itoa(fields.size()).Str("] = {").Str(styles).Str("};");
        code.Eol().NodeName().Function("SetStatusStyles(").itoa(fields.size()).Comma().Str("sb_field_styles);");
        code.CloseBrace();
    }
    else
    {
        code.Eol(eol_if_empty).NodeName().Function("SetStatusWidths([").Str(widths).Str("]").EndFunction();
        code.Eol(eol_if_empty).NodeName().Function("SetStatusStyles([");
        bool is_first_style_set = false;
        for (auto& iter: fields)
        {
            if (is_first_style_set)
                code.Comma();
            else
                is_first_style_set = true;
            code.Add(iter.style);
        }
        code.Str("]").EndFunction();
    }
    return true;
}

int StatusBarGenerator::GetRequiredVersion(Node* node)
{
    if (!node->hasValue(prop_fields))
    {
        return BaseGenerator::GetRequiredVersion(node);
    }
    if (tt::is_digit(node->as_string(prop_fields)[0]))
    {
        return BaseGenerator::GetRequiredVersion(node);
    }
    return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
}

bool StatusBarGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                                     GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/statusbr.h>", set_src, set_hdr);
    return true;
}

int StatusBarGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxStatusBar");

    if (GetRequiredVersion(node) > minRequiredVer)
    {
        auto fields = node->as_statusbar_fields(prop_fields);
        if (fields.size())
        {
            tt_string widths, styles;
            for (auto& iter: fields)
            {
                if (widths.size())
                    widths << ",";
                widths << iter.width;
                if (styles.size())
                    styles << ",";
                styles << iter.style;
            }
            item.append_child("fields").text().set(tt_string() << fields.size());
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
