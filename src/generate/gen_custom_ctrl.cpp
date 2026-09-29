//////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Control generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-29-2026]

#include <tuple>

#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header
#include <wx/stattext.h>          // wxStaticText base header

#include "bitmaps.h"                            // Contains various images handling functions
#include "code.h"                               // Code -- Helper class for generating code
#include "gen_xrc_utils.h"                      // Common XRC generating functions
#include "node.h"                               // Node class
#include "pugixml.hpp"                          // xml read/write/create/process
#include "utils.h"                              // Utility functions that work with properties
#include "write_code.h"                         // WriteCode -- Write code to Scintilla or file
#include "wxue_namespace/wxue_string.h"         // wxue::string, wxue::string_view
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector

#include "gen_custom_ctrl.h"

// Returns true if character can be part of an identifier (letter, digit, or underscore).
static bool IsIdentifierChar(char character)
{
    return wxue::is_alnum(character) || character == '_';
}

// Replaces whole-token occurrences of old_token in text -- a match is only replaced when it is
// not adjacent to an identifier character. This avoids corrupting identifiers such as "myself"
// when replacing "self".
static void ReplaceWholeToken(wxue::string& text, std::string_view old_token,
                              std::string_view new_token)
{
    size_t position = 0;
    while ((position = text.find(old_token, position)) != wxue::npos)
    {
        const size_t match_end = position + old_token.size();
        const bool left_boundary = (position == 0) || !IsIdentifierChar(text[position - 1]);
        const bool right_boundary =
            (match_end >= text.size()) || !IsIdentifierChar(text[match_end]);
        if (left_boundary && right_boundary)
        {
            text.replace(position, old_token.size(), new_token);
            position += new_token.size();
        }
        else
        {
            position = match_end;
        }
    }
}

wxObject* CustomControlGenerator::CreateMockup(Node* node, wxObject* parent)
{
    const wxue::StringVector parts(node->as_string(prop_custom_mockup), ";");
    wxWindow* widget = nullptr;

    if (!parts.empty() && parts[0].starts_with("wxStaticText"))
    {
        if (auto pos = parts[0].find('('); pos != wxue::npos)
        {
            wxue::StringVector options(parts[0].subview(pos + 1), ",");
            // An empty subview yields no elements -- guard the access so we never index an empty
            // vector.
            const wxString label = options.empty() ? wxString() : options[0].wx();
            widget = new wxStaticText(
                wxStaticCast(parent, wxWindow), wxID_ANY, label, wxDefaultPosition, wxDefaultSize,
                wxBORDER_SIMPLE |
                    (options.size() > 1 && options[1].contains("1") ? wxALIGN_CENTER_HORIZONTAL :
                                                                      0));
        }
        else
        {
            widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                      wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
        }
        // NOTE: Keep this sizing logic in sync with the identical block in the bitmap branch
        // below.
        if (parts.size() > 2 && parts[1] != "-1" && parts[2] != "-1")
        {
            widget->SetMinSize(wxSize(parts[1].atoi(), parts[2].atoi()));
        }
        else
        {
            const wxSize size = node->as_wxSize(prop_size);
            if (size.x != -1 && size.y != -1)
            {
                widget->SetMinSize(size);
            }
        }
    }

    // Default to a bitmap if no mockup is specified
    else
    {
        widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY,
                                           GetInternalImage("CustomControl"));
        // NOTE: Keep this sizing logic in sync with the identical block in the wxStaticText
        // branch above.
        if (parts.size() > 2 && parts[1] != "-1" && parts[2] != "-1")
        {
            widget->SetMinSize(wxSize(parts[1].atoi(), parts[2].atoi()));
            wxStaticCast(widget, wxGenericStaticBitmap)->SetScaleMode(wxStaticBitmap::Scale_Fill);
        }
        else
        {
            const wxSize size = node->as_wxSize(prop_size);
            if (size.x != -1 && size.y != -1)
            {
                widget->SetMinSize(size);
                wxStaticCast(widget, wxGenericStaticBitmap)
                    ->SetScaleMode(wxStaticBitmap::Scale_Fill);
            }
        }
    }

    widget->Bind(wxEVT_LEFT_DOWN, &BaseGenerator::OnLeftClick, this);

    return widget;
}

// map_MacroProps is in gen_enums.cpp and provides conversion for ${id}, ${pos}, ${size},
// ${window_extra_style}, ${window_name}, ${window_style}

bool CustomControlGenerator::ConstructionCode(Code& code)
{
    if (code.HasValue(prop_construction))
    {
        wxue::string construction = code.view(prop_construction);
        construction.BothTrim();
        std::ignore = construction.Replace("@@", "\n", wxue::REPLACE::all);
        code += construction;
        return true;
    }

    // A class name is required to construct the control -- without it we would generate an
    // invalid statement such as `auto* m_custom = new (;`, so skip construction entirely.
    if (!code.HasValue(prop_class_name))
    {
        return false;
    }

    code.AddAuto().NodeName();
    code.Str(" = ").AddIfCpp("new ");
    if (code.HasValue(prop_namespace) && code.is_cpp())
    {
        code.as_string(prop_namespace) += "::";
    }

    wxue::string parameters(code.view(prop_parameters));
    if (parameters.starts_with('('))
    {
        parameters.erase(0, 1);
    }
    // Use Code::ValidParentName() rather than Node::get_ParentName() -- the latter returns the
    // form's class name when the parent is the form, instead of the language's self-reference
    // ("this"/"self"). See issue #1869.
    Code parent_code(code.node(), code.get_language());
    parent_code.ValidParentName();
    std::ignore = parameters.Replace("${parent}", parent_code, wxue::REPLACE::all);
    if (code.is_cpp())
    {
        // Replace whole tokens only -- a blanket substring replace would turn "myself" into
        // "mythis".
        ReplaceWholeToken(parameters, "self", "this");
        std::ignore = parameters.Replace("wx.ID_ANY", "wxID_ANY", wxue::REPLACE::all);
    }
    else
    {
        ReplaceWholeToken(parameters, "this", "self");
        std::ignore = parameters.Replace("wxID_ANY", "wx.ID_ANY", wxue::REPLACE::all);
    }

    for (auto& iter: map_MacroProps)
    {
        if (parameters.find(iter.first) != wxue::npos)
        {
            Code code_temp(code.node(), code.get_language());
            if (iter.second == prop_window_style && code.node()->as_string(iter.second).empty())
            {
                // An empty style would leave a dangling '|' in the argument list.
                std::ignore = parameters.Replace(iter.first, "0", wxue::REPLACE::all);
            }
            else if (iter.second == prop_window_extra_style &&
                     code.node()->as_string(iter.second).empty())
            {
                // Same as prop_window_style above -- an empty extra style would leave a
                // dangling '|' in the argument list.
                std::ignore = parameters.Replace(iter.first, "0", wxue::REPLACE::all);
            }
            else if (iter.second == prop_id)
            {
                // Use a language-aware Code object so non-C++ output gets that language's id
                // form (e.g. wx.ID_ANY for Python) instead of the raw C++ identifier.
                Code id_code(code.node(), code.get_language());
                id_code.as_string(prop_id);
                std::ignore = parameters.Replace(iter.first, id_code, wxue::REPLACE::all);
            }
            else if (iter.second == prop_pos)
            {
                const wxPoint pos = code.node()->as_wxPoint(prop_pos);
                code_temp.WxPoint(pos);
                std::ignore = parameters.Replace(iter.first, code_temp, wxue::REPLACE::all);
            }
            else if (iter.second == prop_size)
            {
                const wxSize size = code.node()->as_wxSize(prop_size);
                code_temp.WxSize(size);
                std::ignore = parameters.Replace(iter.first, code_temp, wxue::REPLACE::all);
            }
            else
            {
                // In C++ we can just replace the macro with the string from the property, but in
                // Python we need to do additional processing on most strings.
                if (code.is_cpp())
                {
                    std::ignore =
                        parameters.Replace(iter.first, code.view(iter.second), wxue::REPLACE::all);
                }
                else
                {
                    Code macro(code.node(), code.get_language());
                    macro.Add(code.view(iter.second));
                    std::ignore = parameters.Replace(iter.first, macro, wxue::REPLACE::all);
                }
            }
        }
    }

    if (parameters.empty() || parameters.back() != ')')
    {
        // Always emit a closing parenthesis -- otherwise an empty parameter list would generate
        // an unbalanced "ClassName(".
        parameters += ")";
    }

    code.as_string(prop_class_name)
        .Str("(")
        .CheckLineLength(parameters.size())
        .Str(parameters)
        .AddIfCpp(";");

    return true;
}

bool CustomControlGenerator::SettingsCode(Code& code)
{
    if (code.HasValue(prop_settings_code))
    {
        // Unless the code is fairly simple, it's not really practical to have one settings
        // section that works for both C++ and Python. We do, however, make some basic
        // conversions.

        wxue::string settings = code.view(prop_settings_code);
        std::ignore = settings.Replace("@@", "\n", wxue::REPLACE::all);
        // NOTE: These are whole-block substring replacements, so the settings block must not
        // contain these tokens inside string literals or comments.
        if (code.is_python())
        {
            std::ignore = settings.Replace("->", ".", wxue::REPLACE::all);
            std::ignore = settings.Replace("wxID_ANY", "wx.ID_ANY", wxue::REPLACE::all);
        }
        else if (code.is_cpp())
        {
            std::ignore = settings.Replace("wx.", "wx", wxue::REPLACE::all);
        }
        // Ruby and the FFI languages receive the settings block unchanged.

        code.Str(settings);
    }

    return true;
}

int CustomControlGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    const int result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                        BaseGenerator::xrc_updated;
    pugi::xml_node item = InitializeXrcObject(node, object);

    // NOTE: XRC output is not supported for CustomControl (SUPPORTED.md lists XRC as "---").
    // "unknown" is an intentional placeholder class name -- it is emitted only so the XRC
    // document stays well-formed, and it is not a valid wxWidgets class name.
    GenXrcObjectAttributes(node, item, "unknown");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    return result;
}

bool CustomControlGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                         std::set<std::string>& set_hdr, GenLang language)
{
    if (node->HasValue(prop_header) && language == GenLang::cplusplus)
    {
        // A '#' prefixed value is emitted verbatim -- it may be an #include directive or
        // something else such as #pragma once. Wrapping a non-include directive in
        // #include "..." would generate invalid code.
        const wxue::string_view cur_value = node->as_string(prop_header);
        if (cur_value.starts_with("#"))
        {
            wxue::string convert(node->as_string(prop_header));
            std::ignore = convert.Replace("@@", "\n", wxue::REPLACE::all);
            set_src.insert(convert);
        }
        else
        {
            // Because the header is now a multi-line editor, it's easy for it to have a
            // trailing @@ -- we remove that here.
            wxue::string convert(node->as_string(prop_header));
            std::ignore = convert.Replace("@@", "", wxue::REPLACE::all);

            wxString include_str;
            include_str << "#include \"" << convert << '"';
            set_src.insert(include_str.ToStdString());
        }
    }

    // The forward-declaration/namespace block below is C++-only syntax -- no other language
    // should receive it.
    if (language == GenLang::cplusplus && node->as_string(prop_class_access) != "none" &&
        node->HasValue(prop_class_name))
    {
        if (node->HasValue(prop_namespace))
        {
            wxString hdr_str;
            hdr_str << "namespace " << node->as_string(prop_namespace) << "\n{\n"
                    << "class " << node->as_string(prop_class_name) << ";\n}";
            set_hdr.insert(hdr_str.ToStdString());
        }
        else
        {
            wxString hdr_str;
            hdr_str << "class " << node->as_string(prop_class_name) << ';';
            set_hdr.insert(hdr_str.ToStdString());
        }
    }
    return true;
}
