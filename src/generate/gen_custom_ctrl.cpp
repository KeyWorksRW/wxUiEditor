//////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Control generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

wxObject* CustomControl::CreateMockup(Node* node, wxObject* parent)
{
    const wxue::StringVector parts(node->as_string(prop_custom_mockup), ";");
    wxWindow* widget = nullptr;

    if (parts.size() && parts[0].starts_with("wxStaticText"))
    {
        if (auto pos = parts[0].find('('); pos != wxue::npos)
        {
            wxue::StringVector options(parts[0].subview(pos + 1), ",");
            widget =
                new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, options[0],
                                 wxDefaultPosition, wxDefaultSize,
                                 wxBORDER_SIMPLE | (options.size() > 1 && options[1].contains("1") ?
                                                        wxALIGN_CENTER_HORIZONTAL :
                                                        0));
        }
        else
        {
            widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString,
                                      wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
        }
        if (parts.size() > 2 && parts[1] != "-1" && parts[2] != "-1")
        {
            widget->SetMinSize(wxSize(parts[1].atoi(), parts[2].atoi()));
        }
        else
        {
            auto size = node->as_wxSize(prop_size);
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
        if (parts.size() > 2 && parts[1] != "-1" && parts[2] != "-1")
        {
            widget->SetMinSize(wxSize(parts[1].atoi(), parts[2].atoi()));
            wxStaticCast(widget, wxGenericStaticBitmap)->SetScaleMode(wxStaticBitmap::Scale_Fill);
        }
        else
        {
            auto size = node->as_wxSize(prop_size);
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

bool CustomControl::ConstructionCode(Code& code)
{
    if (code.HasValue(prop_construction))
    {
        wxue::string construction = code.view(prop_construction);
        construction.BothTrim();
        construction.Replace("@@", "\n", wxue::REPLACE::all);
        code += construction;
        return true;
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
    parameters.Replace("${parent}", code.node()->get_ParentName(code.get_language(), true),
                       wxue::REPLACE::all);
    if (code.is_cpp())
    {
        parameters.Replace("self", "this", wxue::REPLACE::all);
        parameters.Replace("wx.ID_ANY", "wxID_ANY", wxue::REPLACE::all);
    }
    else
    {
        parameters.Replace("this", "self", wxue::REPLACE::all);
        parameters.Replace("wxID_ANY", "wx.ID_ANY", wxue::REPLACE::all);
    }

    for (auto& iter: map_MacroProps)
    {
        if (parameters.find(iter.first) != wxue::npos)
        {
            Code code_temp(code.node(), code.get_language());
            if (iter.second == prop_window_style && code.node()->as_string(iter.second).empty())
            {
                parameters.Replace(iter.first, "0");
            }
            else if (iter.second == prop_id)
            {
                parameters.Replace(iter.first, code.node()->get_PropId());
            }
            else if (iter.second == prop_pos)
            {
                auto pos = code.node()->as_wxPoint(prop_pos);
                code_temp.WxPoint(pos);
                parameters.Replace(iter.first, code_temp);
            }
            else if (iter.second == prop_size)
            {
                auto size = code.node()->as_wxSize(prop_size);
                code_temp.WxSize(size);
                parameters.Replace(iter.first, code_temp);
            }
            else
            {
                // In C++ we can just replace the macro with the string from the property, but in
                // Python we need to do additional processing on most strings.
                if (code.is_cpp())
                {
                    parameters.Replace(iter.first, code.view(iter.second));
                }
                else
                {
                    Code macro(code.node(), code.get_language());
                    macro.Add(code.view(iter.second));
                    parameters.Replace(iter.first, macro);
                }
            }
        }
    }

    if (parameters.size() && parameters.back() != ')')
    {
        parameters += ")";
    }

    code.as_string(prop_class_name)
        .Str("(")
        .CheckLineLength(parameters.size())
        .Str(parameters)
        .AddIfCpp(";");

    return true;
}

bool CustomControl::SettingsCode(Code& code)
{
    if (code.HasValue(prop_settings_code))
    {
        // Unless the code is fairly simple, it's not really practical to have one settings
        // section that works for both C++ and Python. We do, however, make some basic
        // conversions.

        wxue::string settings = code.view(prop_settings_code);
        settings.Replace("@@", "\n", wxue::REPLACE::all);
        if (code.is_python())
        {
            settings.Replace("->", ".", wxue::REPLACE::all);
            settings.Replace("wxID_ANY", "wx.ID_ANY", wxue::REPLACE::all);
        }
        else
        {
            settings.Replace("wx.", "wx", wxue::REPLACE::all);
        }

        code.Str(settings);
    }

    return true;
}

int CustomControl::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto result = node->get_Parent()->is_Sizer() ? BaseGenerator::xrc_sizer_item_created :
                                                   BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "unknown");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    return result;
}

bool CustomControl::GetIncludes(Node* node, std::set<std::string>& set_src,
                                std::set<std::string>& set_hdr, GenLang language)
{
    if (node->HasValue(prop_header) && language == GEN_LANG_CPLUSPLUS)
    {
        wxue::string_view cur_value = node->as_string(prop_header);
        if (cur_value.starts_with("#"))
        {
            cur_value.remove_prefix(1);
            cur_value = cur_value.view_nonspace();
            if (cur_value.starts_with("include"))
            {
                wxue::string convert(node->as_string(prop_header));
                convert.Replace("@@", "\n", wxue::REPLACE::all);
                set_src.insert(convert);
            }
            else
            {
                wxString include_str;
                include_str << "#include \"" << node->as_string(prop_header) << '"';
                set_src.insert(include_str.ToStdString());
            }
        }
        else
        {
            // Because the header is now a multi-line editor, it's easy for it to have a
            // trailing @@ -- we remove that here.
            wxue::string convert(node->as_string(prop_header));
            convert.Replace("@@", "", wxue::REPLACE::all);

            wxString include_str;
            include_str << "#include \"" << convert << '"';
            set_src.insert(include_str.ToStdString());
        }
    }

    if (node->as_string(prop_class_access) != "none" && node->HasValue(prop_class_name))
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
