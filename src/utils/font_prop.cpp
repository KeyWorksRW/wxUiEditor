/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

/*

   This class is responsible for all font conversions between wxFont, prop_font, etc.

   Up through version 12 of projects, the property is saved as:

        face name, style, weight, point size, family, underlined, encoding

   With the exception of the facename, all the comma-separated parameters are optional.

   Starting with version 13, if the first parameter starts with "wx" then it is assumed to specify: a family. In this case,
   the parameter order is:

        family, point size, style, weight, underlined

   The reason for making the point size second, is because it's the most likely to be changed, and rest will often simply be
   unused

   Note the lack of facename of encoding. Both facename and encoding are platform-dependent.

*/

#include <cstdlib>  // for std::atof

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "font_prop.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "node_prop.h"     // NodeProperty class

namespace font
{
    enum
    {
        // use idx_wx when working with a system font
        idx_wx_family,
        idx_wx_point,
        idx_wx_style,
        idx_wx_weight,
        idx_wx_underlined,
        idx_wx_strikethrough,

        // Use these when working with a custom font
        idx_facename = 0,
        idx_style,
        idx_weight,
        idx_point,
        idx_family,
        idx_underlined,
        idx_strikethrough,
    };

};

FontProperty::FontProperty(const wxFont& font)
{
    if (font.IsOk())
    {
        Family(font.GetFamily()).Style(font.GetStyle()).Weight(font.GetWeight()).Underlined(font.GetUnderlined());
        if (font.GetFaceName().size())
        {
            FaceName(font.GetFaceName());
        }
    }
}

FontProperty::FontProperty(wxVariant font)
{
    Convert(ttlib::cstr() << font.GetString().wx_str());
}

FontProperty::FontProperty(ttlib::cview font)
{
    Convert(font);
}

FontProperty::FontProperty(NodeProperty* prop)
{
    Convert(prop->as_string());
}

void FontProperty::Convert(ttlib::cview font)
{
    if (font.empty())
    {
        auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
        Family(def_gui.GetFamily()).FaceName(def_gui.GetFaceName()).Style(def_gui.GetStyle()).Weight(def_gui.GetWeight());
        m_pointSize = def_gui.GetFractionalPointSize();

        return;
    }

    // face name, style, weight, point size, family, underlined, aliased, encoding
    //      or
    // family, point size, style, weight, underlined

    ttlib::multiview mstr(font, ',');

    // If font was empty, then we would have already returned, so we know that mstr[0] is valid.
    bool is_system_font = mstr[0].is_sameprefix("wx");

    if (is_system_font)
    {
        m_isDefGuiFont = (mstr[0] == "wxSYS_DEFAULT_GUI_FONT");
        if (mstr.size() > font::idx_wx_point)
        {
            m_isNonDefSize = true;
            m_pointSize = std::atof(std::string(mstr[font::idx_wx_point]).c_str());
        }

        auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
        Family(def_gui.GetFamily()).FaceName(def_gui.GetFaceName()).Style(def_gui.GetStyle()).Weight(def_gui.GetWeight());
    }
    else
    {
        m_isDefGuiFont = false;
        if (mstr.size() > font::idx_point)
        {
            m_pointSize = std::atof(std::string(mstr[font::idx_point]).c_str());
        }
    }

    if (is_system_font)
    {
        auto value = g_NodeCreator.GetConstantAsInt(std::string(mstr[0]));
        if (value >= wxFONTFAMILY_DEFAULT && value < wxFONTFAMILY_MAX)
        {
            Family(static_cast<wxFontFamily>(value));
        }

        if (mstr.size() > font::idx_wx_style)
        {
            auto style = mstr[font::idx_wx_style].atoi();
            if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
            {
                Style(static_cast<wxFontStyle>(style));
            }
        }

        if (mstr.size() > font::idx_wx_weight)
        {
            auto weight = mstr[font::idx_wx_weight].atoi();
            if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
            {
                Weight(static_cast<wxFontWeight>(weight));
            }
        }

        if (mstr.size() > font::idx_wx_underlined)
        {
            Underlined(mstr[font::idx_wx_underlined].atoi() != 0);
        }

        return;
    }
    else
    {
        FaceName(mstr[0].wx_str());
    }

    if (mstr.size() > font::idx_style)
    {
        auto style = mstr[font::idx_style].atoi();
        if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
        {
            Style(static_cast<wxFontStyle>(style));
        }
    }

    if (mstr.size() > font::idx_weight)
    {
        auto weight = mstr[font::idx_weight].atoi();
        if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
        {
            Weight(static_cast<wxFontWeight>(weight));
        }
    }

    if (mstr.size() > font::idx_family)
    {
        auto value = mstr[font::idx_family].atoi();
        if (value >= wxFONTFAMILY_DEFAULT && value < wxFONTFAMILY_MAX)
        {
            Family(static_cast<wxFontFamily>(value));
        }
    }

    if (mstr.size() > font::idx_underlined)
    {
        Underlined(mstr[font::idx_underlined].atoi() != 0);
    }
}

wxFont FontProperty::GetFont() const
{
    wxFontInfo info(m_pointSize);
    info.Family(GetFamily()).Style(GetStyle()).Weight(GetNumericWeight()).FaceName(GetFaceName());
    info.Underlined(IsUnderlined()).Strikethrough(IsStrikethrough());
    return wxFont(info);
}

wxString FontProperty::as_wxString() const
{
    wxString str;
    if (m_isDefGuiFont)
    {
        if (m_isNonDefSize)
        {
            str.Format("wxSYS_DEFAULT_GUI_FONT, %f", GetFractionalPointSize());
        }
        else
        {
            str = "wxSYS_DEFAULT_GUI_FONT";
        }
    }
    else if (GetFaceName().empty())
    {
        str.Format("%d,%f,%d,%d,%d,%d", GetFamily(), GetFractionalPointSize(), GetStyle(), GetWeight(),
                   (IsUnderlined() ? 1 : 0), (IsStrikethrough() ? 1 : 0));
    }
    else  // facename specified
    {
        str.Format("%s,%d,%f,%d,%d,%d,%d", GetFaceName(), GetFamily(), GetFractionalPointSize(), GetStyle(), GetWeight(),
                   (IsUnderlined() ? 1 : 0), (IsStrikethrough() ? 1 : 0));
    }
    return str;
}

ttlib::cstr FontProperty::as_string() const
{
    ttlib::cstr str(as_wxString().wx_str());
    return str;
}
