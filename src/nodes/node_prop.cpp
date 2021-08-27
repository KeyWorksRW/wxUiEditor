/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <array>
#include <charconv>

#include <wx/animate.h>                // wxAnimation and wxAnimationCtrl
#include <wx/propgrid/propgriddefs.h>  // wxPropertyGrid miscellaneous definitions

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "font_prop.h"     // FontProperty -- FontProperty class
#include "mainapp.h"       // App -- App class
#include "mainapp.h"       // App -- Main application class
#include "node.h"          // Node -- Node class
#include "node_creator.h"  // NodeCreator class
#include "pjtsettings.h"   // ProjectSettings -- Hold data for currently loaded project
#include "utils.h"         // Utility functions that work with properties

#include "node_prop.h"

using namespace GenEnum;

NodeProperty::NodeProperty(PropDeclaration* info, Node* node) : m_declaration(info), m_node(node) {}

// The advantage of placing the one-line calls to PropDeclaration (m_declaration) here is that it reduces the header-file
// dependency for other modeuls that need NodeProperty, and it allows for changes to PropDeclaration that don't require
// recompiling every module that included prop_decl.h.

bool NodeProperty::IsDefaultValue() const
{
    return m_value.is_sameas(m_declaration->GetDefaultValue());
}

int NodeProperty::as_int() const
{
    switch (type())
    {
        case type_editoption:
        case type_option:
        case type_id:
            return g_NodeCreator.GetConstantAsInt(m_value, 0);

        case type_bitlist:
            {
                int result = 0;
                ttlib::multistr mstr(m_value, '|');
                for (auto& iter: mstr)
                {
                    result |= g_NodeCreator.GetConstantAsInt(iter);
                }
                return result;
            }

        default:
            return m_value.atoi();  // this will return 0 if the m_value is an empty string
    }
}

wxPoint NodeProperty::as_point() const
{
    wxPoint result { -1, -1 };
    if (m_value.size())
    {
        ttlib::multiview tokens(m_value, ',');
        if (tokens.size())
        {
            if (tokens[0].size())
                result.x = tokens[0].atoi();

            if (tokens.size() > 1 && tokens[1].size())
                result.y = tokens[1].atoi();
        }
    }
    return result;
}

wxSize NodeProperty::as_size() const
{
    wxSize result { -1, -1 };
    if (m_value.size())
    {
        ttlib::multiview tokens(m_value, ',');
        if (tokens.size())
        {
            if (tokens[0].size())
                result.x = tokens[0].atoi();

            if (tokens.size() > 1 && tokens[1].size())
                result.y = tokens[1].atoi();
        }
    }
    return result;
}

wxColour NodeProperty::as_color() const
{
    // check for system colour
    if (m_value.is_sameprefix("wx"))
    {
        return wxSystemSettings::GetColour(ConvertToSystemColour(m_value));
    }
    else
    {
        ttlib::multistr mstr(m_value, ',');
        unsigned long rgb = 0;
        if (mstr.size() > 2)
        {
            auto blue = mstr[2].atoi();
            if (blue < 0 || blue > 255)
                blue = 0;
            rgb |= (blue << 16);
        }
        if (mstr.size() > 1)
        {
            auto green = mstr[1].atoi();
            if (green < 0 || green > 255)
                green = 0;
            rgb |= (green << 8);
        }
        if (mstr.size() > 0)
        {
            auto red = mstr[0].atoi();
            if (red < 0 || red > 255)
                red = 0;
            rgb |= red;
        }
        wxColour clr(rgb);
        return clr;
    }
}

wxFont NodeProperty::as_font() const
{
    return FontProperty(m_value.subview()).GetFont();
}

FontProperty NodeProperty::as_font_prop() const
{
    FontProperty font_prop(m_value.subview());
    return font_prop;
}

wxBitmap NodeProperty::as_bitmap() const
{
    auto image = wxGetApp().GetImage(m_value);
    if (!image.IsOk())
        return wxNullBitmap;
    else
        return image;
}

wxAnimation NodeProperty::as_animation() const
{
    return wxGetApp().GetProjectSettings()->GetPropertyAnimation(m_value);
}

ttlib::cstr NodeProperty::as_escape_text() const
{
    ttlib::cstr result;

    for (auto ch: m_value)
    {
        switch (ch)
        {
            case '\n':
                result += "\\n";
                break;

            case '\t':
                result += "\\t";
                break;

            case '\r':
                result += "\\r";
                break;

            case '\\':
                result += "\\\\";
                break;

            default:
                result += ch;
                break;
        }
    }

    return result;
}

std::vector<ttlib::cstr> NodeProperty::as_vector() const
{
    std::vector<ttlib::cstr> array;
    if (m_value.empty())
        return array;
    ttlib::cstr parse;
    std::string_view value = m_value;
    auto pos = parse.ExtractSubString(value);
    array.emplace_back(parse);

    for (value = ttlib::stepover(value.data() + pos); value.size(); value = ttlib::stepover(value.data() + pos))
    {
        pos = parse.ExtractSubString(value);
        array.emplace_back(parse);
    }

    return array;
}

wxArrayString NodeProperty::as_wxArrayString() const
{
    wxArrayString result;

    if (m_value.empty())
        return result;

    wxString str = wxString::FromUTF8(m_value);
    WX_PG_TOKENIZER2_BEGIN(str, '"')

    result.Add(token);

    WX_PG_TOKENIZER2_END()

    return result;
}

double NodeProperty::as_float() const
{
    double result;

    if (auto [p, ec] = std::from_chars(m_value.data(), m_value.data() + m_value.size(), result); ec == std::errc())
        return result;
    else
        return 0;
}

void NodeProperty::set_value(double value)
{
    std::array<char, 20> str;
    if (auto [ptr, ec] = std::to_chars(str.data(), str.data() + str.size(), value); ec == std::errc())
    {
        m_value.assign(str.data(), ptr);
    }
    else
    {
        m_value.clear();
    }
}

void NodeProperty::set_value(const wxColour& value)
{
    m_value.clear();
    m_value << value.Red() << ',' << value.Green() << ',' << value.Blue();
}

void NodeProperty::set_value(const wxPoint& value)
{
    m_value.clear();
    m_value << value.x << ',' << value.y;
}

void NodeProperty::set_value(const wxSize& value)
{
    m_value.clear();
    m_value << value.x << ',' << value.y;
}

void NodeProperty::set_value(const wxString& value)
{
    m_value.clear();
    m_value << value.wx_str();
}

bool NodeProperty::HasValue() const
{
    if (m_value.empty())
        return false;

    switch (type())
    {
        case type_wxSize:
            return (as_size() != wxDefaultSize);

        case type_wxPoint:
            return (as_point() != wxDefaultPosition);

        case type_animation:
            if (auto semicolonIndex = m_value.find_first_of(";"); ttlib::is_found(semicolonIndex))
            {
                return (semicolonIndex != 0);
            }
            return m_value.size();

        case type_image:
            if (auto semicolonIndex = m_value.find_first_of(";"); ttlib::is_found(semicolonIndex))
            {
                return (semicolonIndex != 0 && semicolonIndex + 2 < m_value.size());
            }
            return m_value.size();

        default:
            return true;
    }
}
