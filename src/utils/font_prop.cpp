/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

#include <charconv>  // for std::to_chars
#include <cstdlib>   // for std::atof

#include "ttmultistr.h"  // multistr -- Breaks a single string into multiple strings

#include "font_prop.h"

#include "node_creator.h"  // NodeCreator -- Class used to create nodes
#include "node_prop.h"     // NodeProperty class

namespace font
{
    enum
    {
        // use idx_gui... when working with a default GUI font
        idx_gui_symbol_size = 0,
        idx_gui_style,
        idx_gui_weight,
        idx_gui_underlined,
        idx_gui_strikethrough,

        // use idx_family... when working with a family font
        idx_family_family = 0,
        idx_family_point,
        idx_family_style,
        idx_family_weight,
        idx_family_underlined,
        idx_family_strikethrough,

        // Use idx_facename... when working with a font that has a non-default facename
        idx_facename = 0,
        idx_facename_style,
        idx_facename_weight,
        idx_facename_point,
        idx_facename_family,
        idx_facename_underlined,
        idx_facename_strikethrough,
    };

};

// All the pairs are declared in font_prop.h

FontSymbolPairs font_symbol_pairs;
FontFamilyPairs font_family_pairs;
FontWeightPairs font_weight_pairs;
FontStylePairs font_style_pairs;

FontProperty::FontProperty()
{
    auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
    Family(def_gui.GetFamily()).FaceName(def_gui.GetFaceName()).Style(def_gui.GetStyle()).Weight(def_gui.GetWeight());
    m_pointSize = def_gui.GetFractionalPointSize();
}

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

// Convert needs to support reading 4 variants:
//
// wxFormBuilder style, wxUiEditor 12 and earlier
//     face name, style, weight, point size, family, underlined
//
// Default GUI font
//     symbol size, style, weight, underlined, strikethrough
//
// wxFontFamily font (facename is blank, point is a floating-point number)
//     family, point size, style, weight, underlined, strikethrough
//
// facename font (point is a floating-point number)
//     facename, point size, family, style, weight, underlined, strikethrough

void FontProperty::Convert(ttlib::cview font)
{
    if (font.empty())
    {
        m_isDefGuiFont = true;
        auto def_gui = wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT);
        Family(def_gui.GetFamily()).FaceName(def_gui.GetFaceName()).Style(def_gui.GetStyle()).Weight(def_gui.GetWeight());
        m_pointSize = def_gui.GetFractionalPointSize();

        return;
    }

    ttlib::multiview mstr(font, ',');

    // If font was empty, then we would have already returned, so we know that mstr[0] is valid.

    if (font_symbol_pairs.HasName(mstr[0]))
    {
        m_isDefGuiFont = true;
        SymbolicSize(font_symbol_pairs.GetValue(mstr[0]));

        if (mstr.size() > font::idx_gui_style)
        {
            Style(font_style_pairs.GetValue(mstr[font::idx_gui_style]));
        }

        if (mstr.size() > font::idx_gui_weight)
        {
            Weight(font_weight_pairs.GetValue(mstr[font::idx_gui_weight]));
        }

        if (mstr.size() > font::idx_gui_underlined)
        {
            Underlined(mstr[font::idx_gui_underlined].is_sameas("underlined"));
        }

        if (mstr.size() > font::idx_gui_strikethrough)
        {
            Strikethrough(mstr[font::idx_gui_strikethrough].is_sameas("strikethrough"));
        }

        return;
    }

    if (font_family_pairs.HasName(mstr[0]))
    {
        m_isDefGuiFont = false;
        FaceName("default");
        Family(font_family_pairs.GetValue(mstr[0]));
        if (mstr.size() > font::idx_family_point)
        {
            m_pointSize = std::atof(std::string(mstr[font::idx_family_point]).c_str());
        }

        if (mstr.size() > font::idx_family_style)
        {
            Style(font_style_pairs.GetValue(mstr[font::idx_family_style]));
        }

        if (mstr.size() > font::idx_family_weight)
        {
            Weight(font_weight_pairs.GetValue(mstr[font::idx_family_weight]));
        }

        if (mstr.size() > font::idx_family_underlined)
        {
            Underlined(mstr[font::idx_family_underlined].is_sameas("underlined"));
        }

        if (mstr.size() > font::idx_family_strikethrough)
        {
            Strikethrough(mstr[font::idx_family_strikethrough].is_sameas("strikethrough"));
        }

        return;
    }

    m_isDefGuiFont = false;
    FaceName(mstr[0].wx_str());

    // We have a facename, so now we need to determine if this is the new style that uses friendly names, or the old
    // wxFB-like style which used numbers. The second value for the wxFB-style is the font style which will be 90 or higher
    // -- too high to be a point size. So, we look at that number, and if it's less than 90, then assume it's the new style.

    if (mstr.size() > font::idx_facename_style)
    {
        auto value = std::atof(std::string(mstr[font::idx_facename_style]).c_str());
        if (value < wxFONTSTYLE_NORMAL)  // wxFONTSTYLE_NORMAL == 90, so too large to be a point size
        {
            m_pointSize = value;

            // With the new style, point size, style and weight have different indexes.

            if (mstr.size() > font::idx_facename_style + 1)
            {
                Style(font_style_pairs.GetValue(mstr[font::idx_facename_style + 1]));
            }

            if (mstr.size() > font::idx_facename_weight + 1)
            {
                Weight(font_weight_pairs.GetValue(mstr[font::idx_facename_weight + 1]));
            }

            if (mstr.size() > font::idx_facename_underlined)
            {
                Underlined(mstr[font::idx_facename_underlined].is_sameas("underlined"));
            }

            if (mstr.size() > font::idx_facename_strikethrough)
            {
                Strikethrough(mstr[font::idx_facename_strikethrough].is_sameas("strikethrough"));
            }

            return;
        }
        else
        {
            m_pointSize = std::atof(std::string(mstr[font::idx_facename_point]).c_str());
        }
    }

    // If we get here, this is an old-style and/or wxFormBuilder property

    if (mstr.size() > font::idx_facename_style)
    {
        auto style = mstr[font::idx_facename_style].atoi();
        if (style >= wxFONTSTYLE_NORMAL && style < wxFONTSTYLE_MAX)
        {
            Style(static_cast<wxFontStyle>(style));
        }
    }

    if (mstr.size() > font::idx_facename_weight)
    {
        auto weight = mstr[font::idx_facename_weight].atoi();
        if (weight >= wxFONTWEIGHT_NORMAL && weight < wxFONTWEIGHT_MAX)
        {
            Weight(static_cast<wxFontWeight>(weight));
        }
    }

    if (mstr.size() > font::idx_facename_family)
    {
        auto value = mstr[font::idx_facename_family].atoi();
        if (value >= wxFONTFAMILY_DEFAULT && value < wxFONTFAMILY_MAX)
        {
            Family(static_cast<wxFontFamily>(value));
        }
    }

    if (mstr.size() > font::idx_facename_underlined)
    {
        Underlined(mstr[font::idx_facename_underlined].atoi() != 0);
    }

    if (mstr.size() > font::idx_facename_strikethrough)
    {
        Strikethrough(mstr[font::idx_facename_strikethrough].atoi() != 0);
    }
}

wxString FontProperty::as_wxString() const
{
    wxString str;
    if (m_isDefGuiFont)
    {
        // symbol size, style, weight, underlined, strikethrough

        ttlib::cstr prop_str(font_symbol_pairs.GetName(GetSymbolSize()));
        prop_str << "," << (GetStyle() == wxFONTSTYLE_NORMAL ? "" : font_style_pairs.GetName(GetStyle()));
        prop_str << "," << (GetWeight() == wxFONTWEIGHT_NORMAL ? "" : font_weight_pairs.GetName(GetWeight()));
        prop_str.Replace(",normal", ",", true);
        if (!IsUnderlined() && !IsStrikethrough())
        {
            while (prop_str.back() == ',')
                prop_str.pop_back();
            str = prop_str;
            if (str == font_symbol_pairs.GetName(wxFONTSIZE_MEDIUM))
                str.clear();
            return str;
        }
        prop_str << "," << (IsUnderlined() ? "underlined" : "");
        if (!IsStrikethrough())
        {
            str = prop_str;
            return str;
        }
        prop_str.Replace(",normal", ",", true);
        prop_str << ",strikethrough";
        str = prop_str;
    }
    else if (GetFaceName().empty())
    {
        ttlib::cstr prop_str(font_family_pairs.GetName(GetFamily()));
        {
            // std::to_chars will return the smallest number of digits needed to represent the number. Point sizes are
            // usually whole numbers, so most of the time this will return a numnber without a decimal point.

            std::array<char, 10> float_str;
            if (auto [ptr, ec] =
                    std::to_chars(float_str.data(), float_str.data() + float_str.size(), GetFractionalPointSize());
                ec == std::errc())
            {
                prop_str << ',' << std::string_view(float_str.data(), ptr - float_str.data());
            }
            else
            {
                // this should be impossible to reach
                prop_str << ",9";
            }
        }
        prop_str << "," << (GetStyle() == wxFONTSTYLE_NORMAL ? "" : font_style_pairs.GetName(GetStyle()));
        prop_str << "," << (GetWeight() == wxFONTWEIGHT_NORMAL ? "" : font_weight_pairs.GetName(GetWeight()));
        if (!IsUnderlined() && !IsStrikethrough())
        {
            while (prop_str.back() == ',')
                prop_str.pop_back();
            str = prop_str;
            return str;
        }
        prop_str << "," << (IsUnderlined() ? "underlined" : "");
        if (!IsStrikethrough())
        {
            str = prop_str;
            return str;
        }
        prop_str << ",strikethrough";
        str = prop_str;
    }
    else  // facename specified
    {
        ttlib::cstr prop_str(GetFaceName());
        {
            std::array<char, 10> float_str;
            if (auto [ptr, ec] =
                    std::to_chars(float_str.data(), float_str.data() + float_str.size(), GetFractionalPointSize());
                ec == std::errc())
            {
                prop_str << ',' << std::string_view(float_str.data(), ptr - float_str.data());
            }
            else
            {
                // this should be impossible to reach
                prop_str << ",9";
            }
        }
        prop_str << "," << (GetStyle() == wxFONTSTYLE_NORMAL ? "" : font_style_pairs.GetName(GetStyle()));
        prop_str << "," << (GetWeight() == wxFONTWEIGHT_NORMAL ? "" : font_weight_pairs.GetName(GetWeight()));
        prop_str << "," << (GetFamily() == wxFONTFAMILY_DEFAULT ? "" : font_family_pairs.GetName(GetFamily()));
        if (!IsUnderlined() && !IsStrikethrough())
        {
            while (prop_str.back() == ',')
                prop_str.pop_back();
            str = prop_str;
            return str;
        }
        prop_str << "," << (IsUnderlined() ? "underlined" : "");
        if (!IsStrikethrough())
        {
            str = prop_str;
            return str;
        }
        prop_str << ",strikethrough";
        str = prop_str;
    }
    return str;
}

ttlib::cstr FontProperty::as_string() const
{
    ttlib::cstr str(as_wxString().wx_str());
    return str;
}

wxFont FontProperty::GetFont() const
{
    if (m_isDefGuiFont)
    {
        wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
        font.SetSymbolicSize(GetSymbolSize());
        font.SetStyle(GetStyle());
        font.SetWeight(GetWeight());
        font.SetUnderlined(IsUnderlined());
        font.SetStrikethrough(IsStrikethrough());
        return font;
    }
    else
    {
        wxFontInfo info(m_pointSize);
        info.Family(GetFamily()).Style(GetStyle()).Weight(GetNumericWeight()).FaceName(GetFaceName());
        info.Underlined(IsUnderlined()).Strikethrough(IsStrikethrough());
        return wxFont(info);
    }
}
