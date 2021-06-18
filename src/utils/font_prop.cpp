/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "font_prop.h"

FontProperty::FontProperty(const wxFont& font)
{
    if (font.IsOk())
    {
        m_pointSize = font.GetPointSize();
        m_family = font.GetFamily();
        m_style = font.GetStyle();
        m_weight = font.GetWeight();
        m_underlined = font.GetUnderlined();
        m_face = font.GetFaceName();
        m_encoding = font.GetEncoding();
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
    // face name, style, weight, point size, family, underlined, aliased, encoding
    ttlib::multistr mstr(font, ',');

    if (mstr.size() > 0)
    {
        m_face = mstr[0];
    }

    if (mstr.size() > 1)
    {
        auto style = mstr[1].atoi();
        if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
        {
            m_style = static_cast<wxFontStyle>(style);
        }
    }

    if (mstr.size() > 2)
    {
        auto weight = mstr[2].atoi();
        if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
        {
            m_weight = static_cast<wxFontWeight>(weight);
        }
    }

    if (mstr.size() > 3)
    {
        m_pointSize = mstr[3].atoi();
    }

    if (mstr.size() > 4)
    {
        auto family = mstr[4].atoi();
        if (family >= wxFONTFAMILY_DEFAULT && family < wxFONTFAMILY_MAX)
        {
            m_family = static_cast<wxFontFamily>(family);
        }
    }

    if (mstr.size() > 5)
    {
        m_underlined = (mstr[5].atoi() != 0);
    }

    if (mstr.size() > 6)
    {
        m_encoding = static_cast<wxFontEncoding>(mstr[6].atoi());
    }
}

wxFont FontProperty::GetFont() const
{
    wxFontInfo finfo(m_pointSize);
    finfo.Family(m_family).FaceName(m_face).Style(m_style).Weight(m_weight).Encoding(m_encoding);
    // finfo.Family(m_family).Style(m_style).Weight(m_weight).Encoding(m_encoding);
    finfo.Underlined(m_underlined).Strikethrough(m_strikethrough);
    if (m_aliased)
        finfo.AntiAliased(m_aliased);
    return wxFont(finfo);
}

wxString FontProperty::as_wxString() const
{
    wxString str;
    str.Printf("%s,%d,%d,%d,%d,%d", m_face, (int) m_style, (int) m_weight, (int) m_pointSize, (int) m_family, m_underlined ? 1 : 0);
    return str;
}

ttlib::cstr FontProperty::as_string() const
{
    ttlib::cstr str;
    str.Format("%s,%d,%d,%d,%d,%d", m_face.utf8_str().data(), (int) m_style, (int) m_weight, (int) m_pointSize, (int) m_family,
               m_underlined ? 1 : 0);
    return str;
}
