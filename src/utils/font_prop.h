/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <cmath>  // for std::round

#include "wx/font.h"  // wxFontBase class: the interface of wxFont

class wxVariant;
class NodeProperty;

// Storing everything in a wxFontInfo class would be ideal, but that class requires the point size in the constructor and
// doesn't allow you to change it after the fact.

class FontProperty
{
public:
    FontProperty();
    FontProperty(const wxFont& font);
    FontProperty(ttlib::cview font);
    FontProperty(NodeProperty* prop);
    FontProperty(wxVariant font);

    wxFont GetFont() const;

    void Convert(ttlib::cview font);
    wxString as_wxString() const;
    ttlib::cstr as_string() const;

    bool isDefGuiFont() const { return m_isDefGuiFont; }
    void setDefGuiFont(bool use_default = true) { m_isDefGuiFont = use_default; }

    int GetPointSize() const { return std::lround(m_pointSize); }
    double GetFractionalPointSize() const { return m_pointSize; }

    wxFontStyle GetStyle() const
    {
        return m_flags & wxFONTFLAG_ITALIC ? wxFONTSTYLE_ITALIC :
               m_flags & wxFONTFLAG_SLANT  ? wxFONTSTYLE_SLANT :
                                             wxFONTSTYLE_NORMAL;
    }

    wxFontWeight GetWeight() const { return m_weight; }
    wxFontWeight GetNumericWeight() const { return m_weight; }

    wxFontSymbolicSize GetSymbolSize() const { return m_symbolic_size; }

    bool HasFaceName() const { return m_faceName.size(); }
    wxFontFamily GetFamily() const { return m_family; }
    const wxString& GetFaceName() const { return m_faceName; }

    bool IsAntiAliased() const { return (m_flags & wxFONTFLAG_ANTIALIASED) != 0; }
    bool IsUnderlined() const { return (m_flags & wxFONTFLAG_UNDERLINED) != 0; }
    bool IsStrikethrough() const { return (m_flags & wxFONTFLAG_STRIKETHROUGH) != 0; }

    // wxFontInfo uses this, be we don't support using pixel size
    bool IsUsingSizeInPixels() const { return false; }

    wxFontEncoding GetEncoding() const { return m_encoding; }

    operator wxFont() const { return GetFont(); }

    FontProperty& PointSize(double point_size)
    {
        m_pointSize = point_size;
        return *this;
    }

    FontProperty& SymbolicSize(wxFontSymbolicSize symbolic_size)
    {
        m_symbolic_size = symbolic_size;
        return *this;
    }

    // The following setters match the names in wxFontInfo so that code written for wxFontInfo or FontProperty can be used
    // interchangeably (at least for setters and getters)

    FontProperty& Family(wxFontFamily family)
    {
        m_family = family;
        return *this;
    }
    FontProperty& FaceName(const wxString& faceName)
    {
        m_faceName = faceName;
        return *this;
    }

    FontProperty& Weight(wxFontWeight weight)
    {
        m_weight = weight;
        return *this;
    }

    FontProperty& Bold(bool bold = true) { return Weight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL); }
    FontProperty& Light(bool light = true) { return Weight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL); }
    FontProperty& Italic(bool italic = true)
    {
        SetFlag(wxFONTFLAG_ITALIC, italic);
        return *this;
    }
    FontProperty& Slant(bool slant = true)
    {
        SetFlag(wxFONTFLAG_SLANT, slant);
        return *this;
    }
    FontProperty& Style(wxFontStyle style)
    {
        SetFlag(wxFONTFLAG_ITALIC, style == wxFONTSTYLE_ITALIC);
        SetFlag(wxFONTFLAG_SLANT, style == wxFONTSTYLE_SLANT);
        return *this;
    }

    FontProperty& Underlined(bool underlined = true)
    {
        SetFlag(wxFONTFLAG_UNDERLINED, underlined);
        return *this;
    }
    FontProperty& Strikethrough(bool strikethrough = true)
    {
        SetFlag(wxFONTFLAG_STRIKETHROUGH, strikethrough);
        return *this;
    }

    FontProperty& Encoding(wxFontEncoding encoding)
    {
        m_encoding = encoding;
        return *this;
    }

protected:
    wxFontWeight GetWeightClosestToNumericValue(int numWeight) const
    {
        wxASSERT(numWeight > 0);
        wxASSERT(numWeight <= 1000);

        // round to nearest hundredth = wxFONTWEIGHT_ constant
        int weight = ((numWeight + 50) / 100) * 100;

        if (weight < wxFONTWEIGHT_THIN)
            weight = wxFONTWEIGHT_THIN;
        if (weight > wxFONTWEIGHT_MAX)
            weight = wxFONTWEIGHT_MAX;

        return static_cast<wxFontWeight>(weight);
    }

    void SetFlag(int flag, bool on)
    {
        if (on)
            m_flags |= flag;
        else
            m_flags &= ~flag;
    }

private:
    wxFontFamily m_family { wxFONTFAMILY_DEFAULT };
    wxString m_faceName;
    wxFontEncoding m_encoding { wxFONTENCODING_DEFAULT };
    double m_pointSize { wxSystemSettings().GetFont(wxSYS_DEFAULT_GUI_FONT).GetFractionalPointSize() };
    wxFontWeight m_weight { wxFONTWEIGHT_NORMAL };
    int m_flags { wxFONTFLAG_DEFAULT };

    wxFontSymbolicSize m_symbolic_size { wxFONTSIZE_MEDIUM };

    bool m_isDefGuiFont { true };
};

// Friendly name, wxFontSymbolicSize pair
struct FontSymbolPairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontSymbolicSize>> pairs = {
        { "tiny", wxFONTSIZE_XX_SMALL },
        { "extra small", wxFONTSIZE_X_SMALL },
        { "small", wxFONTSIZE_SMALL },
        { "normal", wxFONTSIZE_MEDIUM },
        { "large", wxFONTSIZE_LARGE },
        { "extra large", wxFONTSIZE_X_LARGE },
        { "huge", wxFONTSIZE_XX_LARGE },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    auto GetValue(ttlib::sview name) const
    {
        if (name.empty() || name.is_sameas("normal"))
            return wxFONTSIZE_MEDIUM;

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
                return value;
        }
        return wxFONTSIZE_MEDIUM;
    }

    const std::string& GetKey(wxFontSymbolicSize symbol_size) const
    {
        for (const auto& [key, value]: pairs)
        {
            if (symbol_size == value)
                return key;
        }
        return tt_empty_cstr;
    }
};
extern FontSymbolPairs font_symbol_pairs;

// Friendly name, wxFontFamily pair
struct FontFamilyPairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontFamily>> pairs = {
        { "default", wxFONTFAMILY_DEFAULT },
        { "decorative", wxFONTFAMILY_DECORATIVE },
        { "roman", wxFONTFAMILY_ROMAN },
        { "script", wxFONTFAMILY_SCRIPT },
        { "swiss", wxFONTFAMILY_SWISS },
        { "modern", wxFONTFAMILY_MODERN },
        { "teletype", wxFONTFAMILY_TELETYPE },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    auto GetValue(ttlib::sview name) const
    {
        if (name.empty() || name.is_sameas("default"))
            return wxFONTFAMILY_DEFAULT;

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
                return value;
        }
        return wxFONTFAMILY_DEFAULT;
    }

    const std::string& GetKey(wxFontFamily family) const
    {
        for (const auto& [key, value]: pairs)
        {
            if (family == value)
                return key;
        }
        return tt_empty_cstr;
    }
};
extern FontFamilyPairs font_family_pairs;

// Friendly name, wxFontWeight pair
struct FontWeightPairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontWeight>> pairs = {
        { "thin", wxFONTWEIGHT_THIN },
        { "extra light", wxFONTWEIGHT_EXTRALIGHT },
        { "light", wxFONTWEIGHT_LIGHT },
        { "normal", wxFONTWEIGHT_NORMAL },
        { "medium", wxFONTWEIGHT_MEDIUM },
        { "semi-bold", wxFONTWEIGHT_SEMIBOLD },
        { "bold", wxFONTWEIGHT_BOLD },
        { "extra bold", wxFONTWEIGHT_EXTRABOLD },
        { "heavy", wxFONTWEIGHT_HEAVY },
        { "extra heavy", wxFONTWEIGHT_EXTRAHEAVY },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    auto GetValue(ttlib::sview name) const
    {
        if (name.empty() || name.is_sameas("normal"))
            return wxFONTWEIGHT_NORMAL;

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
                return value;
        }
        return wxFONTWEIGHT_NORMAL;
    }

    const std::string& GetKey(wxFontWeight weight) const
    {
        for (const auto& [key, value]: pairs)
        {
            if (weight == value)
                return key;
        }
        return tt_empty_cstr;
    }
};
extern FontWeightPairs font_weight_pairs;

// Friendly name, wxFontStyle pair
struct FontStylePairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontStyle>> pairs = {
        { "normal", wxFONTSTYLE_NORMAL },
        { "italic", wxFONTSTYLE_ITALIC },
        { "slant", wxFONTSTYLE_SLANT },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    auto GetValue(ttlib::sview name) const
    {
        if (name.empty() || name.is_sameas("normal"))
            return wxFONTSTYLE_NORMAL;

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
                return value;
        }
        return wxFONTSTYLE_NORMAL;
    }

    const std::string& GetKey(wxFontStyle style) const
    {
        for (const auto& [key, value]: pairs)
        {
            if (style == value)
                return key;
        }
        return tt_empty_cstr;
    }
};
extern FontStylePairs font_style_pairs;
