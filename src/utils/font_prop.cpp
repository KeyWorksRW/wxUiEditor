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

        // Use these when working with a custom font
        idx_facename = 0,
        idx_style,
        idx_weight,
        idx_point,
        idx_family,
        idx_underlined,
        idx_encoding
    };

};

FontProperty::~FontProperty()
{
    delete m_info;
}

FontProperty::FontProperty(const wxFont& font)
{
    if (font.IsOk())
    {
        m_info = new wxFontInfo(font.GetFractionalPointSize());
        m_info->Family(font.GetFamily()).Style(font.GetStyle()).Weight(font.GetWeight()).Underlined(font.GetUnderlined());
        if (font.GetFaceName().size())
        {
            m_info->FaceName(font.GetFaceName());
        }
    }
    else
    {
        m_info = new wxFontInfo;
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

void FontProperty::Convert(ttlib::cview font)
{
    if (m_info)
    {
        delete m_info;
        m_info = nullptr;
    }

    if (font.empty())
    {
        m_info = new wxFontInfo;
        auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
        m_info->Family(def_gui.GetFamily())
            .FaceName(def_gui.GetFaceName())
            .Style(def_gui.GetStyle())
            .Weight(def_gui.GetWeight());

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
            if (mstr[font::idx_wx_point].contains("."))
            {
                m_info = new wxFontInfo(std::atof(std::string(mstr[font::idx_wx_point]).c_str()));
            }
            else
            {
                m_info = new wxFontInfo(mstr[font::idx_wx_point].atoi());
            }
        }
        else
        {
            m_info = new wxFontInfo;
        }

        auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
        m_info->Family(def_gui.GetFamily())
            .FaceName(def_gui.GetFaceName())
            .Style(def_gui.GetStyle())
            .Weight(def_gui.GetWeight());
    }
    else
    {
        if (mstr.size() > font::idx_point)
        {
            if (mstr[font::idx_point].contains("."))
            {
                m_info = new wxFontInfo(std::atof(std::string(mstr[font::idx_point]).c_str()));
            }
            else
            {
                m_info = new wxFontInfo(mstr[font::idx_point].atoi());
            }
        }
        else
        {
            m_info = new wxFontInfo;
        }
    }

    // At this point, m_info will be valid and the point size will have been set (or default value is used)

    if (is_system_font)
    {
        auto value = g_NodeCreator.GetConstantAsInt(std::string(mstr[0]));
        if (value >= wxFONTFAMILY_DEFAULT && value < wxFONTFAMILY_MAX)
        {
            m_info->Family(static_cast<wxFontFamily>(value));
        }

        if (mstr.size() > font::idx_wx_style)
        {
            auto style = mstr[font::idx_wx_style].atoi();
            if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
            {
                m_info->Style(static_cast<wxFontStyle>(style));
            }
        }

        if (mstr.size() > font::idx_wx_weight)
        {
            auto weight = mstr[font::idx_wx_weight].atoi();
            if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
            {
                m_info->Weight(static_cast<wxFontWeight>(weight));
            }
        }

        if (mstr.size() > font::idx_wx_underlined)
        {
            m_info->Underlined(mstr[font::idx_wx_underlined].atoi() != 0);
        }

        return;
    }
    else
    {
        m_info->FaceName(mstr[0].wx_str());
    }

    if (mstr.size() > font::idx_style)
    {
        auto style = mstr[font::idx_style].atoi();
        if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
        {
            m_info->Style(static_cast<wxFontStyle>(style));
        }
    }

    if (mstr.size() > font::idx_weight)
    {
        auto weight = mstr[font::idx_weight].atoi();
        if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
        {
            m_info->Weight(static_cast<wxFontWeight>(weight));
        }
    }

    if (mstr.size() > font::idx_family)
    {
        auto value = mstr[font::idx_family].atoi();
        if (value >= wxFONTFAMILY_DEFAULT && value < wxFONTFAMILY_MAX)
        {
            m_info->Family(static_cast<wxFontFamily>(value));
        }
    }

    if (mstr.size() > font::idx_underlined)
    {
        m_info->Underlined(mstr[font::idx_underlined].atoi() != 0);
    }
}

wxFont FontProperty::GetFont() const
{
    return wxFont(*m_info);
}

wxString FontProperty::as_wxString() const
{
    wxString str;
    if (m_isDefGuiFont)
    {
        if (m_isNonDefSize)
        {
            str.Format("wxSYS_DEFAULT_GUI_FONT, %f", m_info->GetFractionalPointSize());
        }
        else
        {
            str = "wxSYS_DEFAULT_GUI_FONT";
        }
    }
    else if (m_info->GetFaceName().empty())
    {
        str.Format("%d,%f,%d,%d,%d", m_info->GetFamily(), m_info->GetFractionalPointSize(), m_info->GetStyle(),
                   m_info->GetWeight(), (m_info->IsUnderlined() ? 1 : 0));
    }
    return str;
}

ttlib::cstr FontProperty::as_string() const
{
    ttlib::cstr str(as_wxString().wx_str());
    return str;
}
