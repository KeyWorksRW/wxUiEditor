//////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Control generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/generic/statbmpg.h>  // wxGenericStaticBitmap header
#include <wx/stattext.h>          // wxStaticText base header

#include "bitmaps.h"        // Contains various images handling functions
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "node.h"           // Node class
#include "pugixml.hpp"      // xml read/write/create/process
#include "utils.h"          // Utility functions that work with properties
#include "write_code.h"     // WriteCode -- Write code to Scintilla or file

#include "gen_custom_ctrl.h"

wxObject* CustomControl::CreateMockup(Node* node, wxObject* parent)
{
    tt_string_vector parts(node->as_string(prop_custom_mockup), ";");
    wxWindow* widget = nullptr;

    if (parts.size() && parts[0].starts_with("wxStaticText"))
    {
        if (auto pos = parts[0].find('('); pos != tt::npos)
        {
            tt_string_vector options(parts[0].subview(pos + 1), ",");
            widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, options[0], wxDefaultPosition, wxDefaultSize,
                                      wxBORDER_SIMPLE |
                                          (options.size() > 1 && options[1].contains("1") ? wxALIGN_CENTER_HORIZONTAL : 0));
        }
        else
        {
            widget = new wxStaticText(wxStaticCast(parent, wxWindow), wxID_ANY, wxEmptyString, wxDefaultPosition,
                                      wxDefaultSize, wxBORDER_SIMPLE);
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
        widget = new wxGenericStaticBitmap(wxStaticCast(parent, wxWindow), wxID_ANY, GetInternalImage("CustomControl"));
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
                wxStaticCast(widget, wxGenericStaticBitmap)->SetScaleMode(wxStaticBitmap::Scale_Fill);
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
    if (code.hasValue(prop_construction))
    {
        tt_string construction = code.view(prop_construction);
        construction.BothTrim();
        construction.Replace("@@", "\n", tt::REPLACE::all);
        code += construction;
        return true;
    }

    code.AddAuto().NodeName();
    code.Str(" = ").AddIfCpp("new ");
    if (code.hasValue(prop_namespace) && code.is_cpp())
        code.as_string(prop_namespace) += "::";

    tt_string parameters(code.view(prop_parameters));
    if (parameters.starts_with('('))
        parameters.erase(0, 1);
    parameters.Replace("${parent}", code.node()->getParentName(), tt::REPLACE::all);
    if (code.is_cpp())
    {
        parameters.Replace("self", "this", tt::REPLACE::all);
        parameters.Replace("wx.ID_ANY", "wxID_ANY", tt::REPLACE::all);
    }
    else
    {
        parameters.Replace("this", "self", tt::REPLACE::all);
        parameters.Replace("wxID_ANY", "wx.ID_ANY", tt::REPLACE::all);
    }

    for (auto& iter: map_MacroProps)
    {
        if (parameters.find(iter.first) != tt::npos)
        {
            Code code_temp(code.node(), code.get_language());
            if (iter.second == prop_window_style && code.node()->as_string(iter.second).empty())
            {
                parameters.Replace(iter.first, "0");
            }
            else if (iter.second == prop_id)
            {
                parameters.Replace(iter.first, code.node()->getPropId());
            }
            else if (iter.second == prop_pos)
            {
                auto pos = code.node()->as_wxPoint(prop_pos);
                if (pos.x == -1 && pos.y == -1)
                    code_temp.Add("wxDefaultPosition");
                else
                    code_temp.Add("wxPoint(") << pos.x << ", " << pos.y << ")";
                parameters.Replace(iter.first, code_temp);
            }
            else if (iter.second == prop_size)
            {
                auto size = code.node()->as_wxSize(prop_size);
                if (size.x == -1 && size.y == -1)
                    code_temp.Add("wxDefaultSize");
                else
                    code_temp.Add("wxSize(") << size.x << ", " << size.y << ")";
                parameters.Replace(iter.first, code_temp);
            }
            else
            {
                // In C++ we can just replace the macro with the string from the property, but in Python we need to
                // do additional processing on most strings.
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

    if (parameters.back() != ')')
        parameters += ")";

    code.as_string(prop_class_name).Str("(").CheckLineLength(parameters.size()).Str(parameters).AddIfCpp(";");

    return true;
}

bool CustomControl::SettingsCode(Code& code)
{
    if (code.hasValue(prop_settings_code))
    {
        // Unless the code is fairly simple, it's not really practical to have one settings
        // section that works for both C++ and Python. We do, however, make some basic
        // conversions.

        tt_string settings = code.view(prop_settings_code);
        settings.Replace("@@", "\n", tt::REPLACE::all);
        if (code.is_python())
        {
            settings.Replace("->", ".", tt::REPLACE::all);
            settings.Replace("wxID_ANY", "wx.ID_ANY", tt::REPLACE::all);
        }
        else
        {
            settings.Replace("wx.", "wx", tt::REPLACE::all);
        }

        code.Str(settings);
    }

    return true;
}

int CustomControl::GenXrcObject(Node* node, pugi::xml_node& object, size_t /* xrc_flags */)
{
    auto result = node->getParent()->isSizer() ? BaseGenerator::xrc_sizer_item_created : BaseGenerator::xrc_updated;
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "unknown");
    GenXrcStylePosSize(node, item);
    GenXrcWindowSettings(node, item);

    return result;
}

bool CustomControl::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr, int language)
{
    if (node->hasValue(prop_header) && language == GEN_LANG_CPLUSPLUS)
    {
        tt_string_view cur_value = node->as_string(prop_header);
        if (cur_value.starts_with("#"))
        {
            cur_value.remove_prefix(1);
            cur_value = cur_value.view_nonspace();
            if (cur_value.starts_with("include"))
            {
                tt_string convert(node->as_string(prop_header));
                convert.Replace("@@", "\n", tt::REPLACE::all);
                set_src.insert(convert);
            }
            else
            {
                set_src.insert(tt_string() << "#include \"" << node->as_string(prop_header) << '"');
            }
        }
        else
        {
            // Because the header is now a multi-line editor, it's easy for it to have a
            // trailing @@ -- we remove that here.
            tt_string convert(node->as_string(prop_header));
            convert.Replace("@@", "", tt::REPLACE::all);

            set_src.insert(tt_string() << "#include \"" << convert << '"');
        }
    }

    if (node->as_string(prop_class_access) != "none" && node->hasValue(prop_class_name))
    {
        if (node->hasValue(prop_namespace))
        {
            set_hdr.insert(tt_string("namespace ") << node->as_string(prop_namespace) << "\n{\n\t"
                                                   << "class " << node->as_string(prop_class_name) << ";\n}");
        }
        else
            set_hdr.insert(tt_string() << "class " << node->as_string(prop_class_name) << ';');
    }
    return true;
}
