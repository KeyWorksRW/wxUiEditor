/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

// AI Context: This file implements FontProperty, a mutable font description class mimicking
// wxFontInfo's fluent API while allowing point size changes and serialization. The class stores
// m_pointSize (double for fractional points), m_family (wxFontFamily), m_faceName (wxString),
// m_weight (wxFontWeight), m_flags (bitfield for italic/ underline/strikethrough/antialiased),
// m_symbolic_size (wxFontSymbolicSize for relative sizing), m_encoding (wxFontEncoding), and
// m_isDefGuiFont (tracks if using system default). Methods provide fluent setters (PointSize,
// Family, Bold, Italic returning *this) and getters (GetFont, GetPointSize, GetStyle). Convert()
// parses serialized font strings (old_style format or new comma-separated), as_string/as_wxString
// serialize to storage format. Helper structs (FontSymbolPairs, FontFamilyPairs, FontWeightPairs,
// FontStylePairs) map friendly names ("bold", "italic") to wxWidgets constants for property grid
// dropdowns, supporting bidirectional conversion for UI and code.

#pragma once

#include <algorithm>  // for std::max, std::min
#include <cmath>      // for std::round

#include <wx/font.h>      // wxFontBase class: the interface of wxFont
#include <wx/settings.h>  // for wxSystemSettings

class wxVariant;
class NodeProperty;

// Storing everything in a wxFontInfo class would be ideal, but that class requires the point size
// in the constructor and doesn't allow you to change it after the fact.

class FontProperty
{
public:
    FontProperty();
    FontProperty(const wxFont& font);
    FontProperty(tt_string_view font);
    FontProperty(NodeProperty* prop);
    FontProperty(wxVariant font);

    [[nodiscard]] auto GetFont() const -> wxFont;

    // If old_style is true, then assume:
    // face name, style, weight, point size, family, underlined
    auto Convert(tt_string_view font, bool old_style = false) -> void;

    [[nodiscard]] auto as_wxString() const -> wxString;
    [[nodiscard]] auto as_string() const -> tt_string;

    [[nodiscard]] auto isDefGuiFont() const -> bool { return m_isDefGuiFont; }
    auto setDefGuiFont(bool use_default = true) -> void { m_isDefGuiFont = use_default; }

    [[nodiscard]] auto GetPointSize() const -> int { return std::lround(m_pointSize); }
    [[nodiscard]] auto GetFractionalPointSize() const -> double { return m_pointSize; }

    [[nodiscard]] auto GetStyle() const -> wxFontStyle
    {
        if (m_flags & wxFONTFLAG_ITALIC)
        {
            return wxFONTSTYLE_ITALIC;
        }
        if (m_flags & wxFONTFLAG_SLANT)
        {
            return wxFONTSTYLE_SLANT;
        }
        return wxFONTSTYLE_NORMAL;
    }

    [[nodiscard]] auto GetWeight() const -> wxFontWeight { return m_weight; }
    [[nodiscard]] auto GetNumericWeight() const -> wxFontWeight { return m_weight; }

    [[nodiscard]] auto GetSymbolSize() const -> wxFontSymbolicSize { return m_symbolic_size; }

    [[nodiscard]] auto HasFaceName() const -> bool { return m_faceName.size(); }
    [[nodiscard]] auto GetFamily() const -> wxFontFamily { return m_family; }
    [[nodiscard]] auto GetFaceName() const -> const wxString& { return m_faceName; }

    [[nodiscard]] auto IsAntiAliased() const -> bool
    {
        return (m_flags & wxFONTFLAG_ANTIALIASED) != 0;
    }
    [[nodiscard]] auto IsUnderlined() const -> bool
    {
        return (m_flags & wxFONTFLAG_UNDERLINED) != 0;
    }
    [[nodiscard]] auto IsStrikethrough() const -> bool
    {
        return (m_flags & wxFONTFLAG_STRIKETHROUGH) != 0;
    }

    // wxFontInfo uses this, be we don't support using pixel size
    [[nodiscard]] static auto IsUsingSizeInPixels() -> bool { return false; }

    [[nodiscard]] auto GetEncoding() const -> wxFontEncoding { return m_encoding; }

    operator wxFont() const { return GetFont(); }

    auto PointSize(double point_size) -> FontProperty&
    {
        m_pointSize = point_size;
        m_isDefGuiFont = false;
        return *this;
    }

    auto SymbolicSize(wxFontSymbolicSize symbolic_size) -> FontProperty&
    {
        m_symbolic_size = symbolic_size;
        return *this;
    }

    // The following setters match the names in wxFontInfo so that code written for wxFontInfo or
    // FontProperty can be used interchangeably (at least for setters and getters)

    auto Family(wxFontFamily family) -> FontProperty&
    {
        m_family = family;
        if (m_family != wxFONTFAMILY_DEFAULT)
        {
            m_isDefGuiFont = false;
        }
        return *this;
    }
    auto FaceName(const wxString& faceName) -> FontProperty&
    {
        m_faceName = faceName;
        if (m_faceName.size())
        {
            m_isDefGuiFont = false;
        }
        return *this;
    }

    auto Weight(wxFontWeight weight) -> FontProperty&
    {
        m_weight = weight;
        return *this;
    }

    auto Bold(bool bold = true) -> FontProperty&
    {
        return Weight(bold ? wxFONTWEIGHT_BOLD : wxFONTWEIGHT_NORMAL);
    }
    auto Light(bool light = true) -> FontProperty&
    {
        return Weight(light ? wxFONTWEIGHT_LIGHT : wxFONTWEIGHT_NORMAL);
    }
    auto Italic(bool italic = true) -> FontProperty&
    {
        SetFlag(wxFONTFLAG_ITALIC, italic);
        return *this;
    }
    auto Slant(bool slant = true) -> FontProperty&
    {
        SetFlag(wxFONTFLAG_SLANT, slant);
        return *this;
    }
    auto Style(wxFontStyle style) -> FontProperty&
    {
        SetFlag(wxFONTFLAG_ITALIC, style == wxFONTSTYLE_ITALIC);
        SetFlag(wxFONTFLAG_SLANT, style == wxFONTSTYLE_SLANT);
        return *this;
    }

    auto Underlined(bool underlined = true) -> FontProperty&
    {
        SetFlag(wxFONTFLAG_UNDERLINED, underlined);
        return *this;
    }
    auto Strikethrough(bool strikethrough = true) -> FontProperty&
    {
        SetFlag(wxFONTFLAG_STRIKETHROUGH, strikethrough);
        return *this;
    }

    auto Encoding(wxFontEncoding encoding) -> FontProperty&
    {
        m_encoding = encoding;
        return *this;
    }

protected:
    [[nodiscard]] static auto GetWeightClosestToNumericValue(int numWeight) -> wxFontWeight
    {
        wxASSERT(numWeight > 0);
        wxASSERT(numWeight <= 1000);

        // round to nearest hundredth = wxFONTWEIGHT_ constant
        constexpr int rounding_offset = 50;
        constexpr int weight_step = 100;
        int weight = ((numWeight + rounding_offset) / weight_step) * weight_step;

        weight = std::max(weight, static_cast<int>(wxFONTWEIGHT_THIN));
        weight = std::min(weight, static_cast<int>(wxFONTWEIGHT_MAX));

        return static_cast<wxFontWeight>(weight);
    }

    auto SetFlag(int flag, bool enable) -> void
    {
        if (enable)
        {
            m_flags |= flag;
        }
        else
        {
            m_flags &= ~flag;
        }
    }

private:
    wxFontFamily m_family { wxFONTFAMILY_DEFAULT };
    wxString m_faceName;
    wxFontEncoding m_encoding { wxFONTENCODING_DEFAULT };
    double m_pointSize {
        wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT).GetFractionalPointSize()
    };
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
        { "normal size", wxFONTSIZE_MEDIUM },
        { "large", wxFONTSIZE_LARGE },
        { "extra large", wxFONTSIZE_X_LARGE },
        { "huge", wxFONTSIZE_XX_LARGE },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    [[nodiscard]] auto GetValue(tt_string_view name) const -> wxFontSymbolicSize
    {
        if (name.empty())
        {
            return wxFONTSIZE_MEDIUM;
        }

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
            {
                return value;
            }
        }
        return wxFONTSIZE_MEDIUM;
    }

    [[nodiscard]] static auto GetValue(wxFontSymbolicSize symbol_size) -> std::string
    {
        switch (symbol_size)
        {
            case wxFONTSIZE_XX_SMALL:
                return "wxFONTSIZE_XX_SMALL";
            case wxFONTSIZE_X_SMALL:
                return "wxFONTSIZE_X_SMALL";
            case wxFONTSIZE_SMALL:
                return "wxFONTSIZE_SMALL";
            case wxFONTSIZE_LARGE:
                return "wxFONTSIZE_LARGE";
            case wxFONTSIZE_X_LARGE:
                return "wxFONTSIZE_X_LARGE";
            case wxFONTSIZE_XX_LARGE:
                return "wxFONTSIZE_XX_LARGE";

            case wxFONTSIZE_MEDIUM:
            default:
                return "wxFONTSIZE_MEDIUM";
        }
    }

    [[nodiscard]] auto HasName(tt_string_view name) const -> bool
    {
        if (name.empty())
        {
            return false;
        }

        return std::ranges::any_of(pairs,
                                   [name](const auto& pair)
                                   {
                                       return name.is_sameas(pair.first);
                                   });
    }

    [[nodiscard]] auto GetName(wxFontSymbolicSize symbol_size) const -> const std::string&
    {
        for (const auto& [key, value]: pairs)
        {
            if (symbol_size == value)
            {
                return key;
            }
        }
        return tt_empty_cstr;
    }
};
extern const FontSymbolPairs font_symbol_pairs;

// Friendly name, wxFontFamily pair
struct FontFamilyPairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontFamily>> pairs = {
        { "default family", wxFONTFAMILY_DEFAULT },
        { "decorative", wxFONTFAMILY_DECORATIVE },
        { "roman", wxFONTFAMILY_ROMAN },
        { "script", wxFONTFAMILY_SCRIPT },
        { "swiss", wxFONTFAMILY_SWISS },
        { "modern", wxFONTFAMILY_MODERN },
        { "teletype", wxFONTFAMILY_TELETYPE },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    [[nodiscard]] auto GetValue(tt_string_view name) const -> wxFontFamily
    {
        if (name.empty())
        {
            return wxFONTFAMILY_DEFAULT;
        }

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
            {
                return value;
            }
        }
        return wxFONTFAMILY_DEFAULT;
    }

    [[nodiscard]] static auto GetValue(wxFontFamily family) -> std::string
    {
        switch (family)
        {
            case wxFONTFAMILY_DECORATIVE:
                return "wxFONTFAMILY_DECORATIVE";
            case wxFONTFAMILY_ROMAN:
                return "wxFONTFAMILY_ROMAN";
            case wxFONTFAMILY_SCRIPT:
                return "wxFONTFAMILY_SCRIPT";
            case wxFONTFAMILY_SWISS:
                return "wxFONTFAMILY_SWISS";
            case wxFONTFAMILY_MODERN:
                return "wxFONTFAMILY_MODERN";
            case wxFONTFAMILY_TELETYPE:
                return "wxFONTFAMILY_TELETYPE";

            case wxFONTFAMILY_DEFAULT:
            default:
                return "wxFONTFAMILY_DEFAULT";
        }
    }

    [[nodiscard]] auto HasName(tt_string_view name) const -> bool
    {
        if (name.empty())
        {
            return false;
        }

        return std::ranges::any_of(pairs,
                                   [name](const auto& pair)
                                   {
                                       return name.is_sameas(pair.first);
                                   });
    }

    [[nodiscard]] auto GetName(wxFontFamily family) const -> const std::string&
    {
        for (const auto& [key, value]: pairs)
        {
            if (family == value)
            {
                return key;
            }
        }
        return tt_empty_cstr;
    }
};
extern const FontFamilyPairs font_family_pairs;

// Friendly name, wxFontWeight pair
struct FontWeightPairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontWeight>> pairs = {
        { "thin", wxFONTWEIGHT_THIN },
        { "extra light", wxFONTWEIGHT_EXTRALIGHT },
        { "light", wxFONTWEIGHT_LIGHT },
        { "normal weight", wxFONTWEIGHT_NORMAL },
        { "medium", wxFONTWEIGHT_MEDIUM },
        { "semi-bold", wxFONTWEIGHT_SEMIBOLD },
        { "bold", wxFONTWEIGHT_BOLD },
        { "extra bold", wxFONTWEIGHT_EXTRABOLD },
        { "heavy", wxFONTWEIGHT_HEAVY },
        { "extra heavy", wxFONTWEIGHT_EXTRAHEAVY },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    [[nodiscard]] auto GetValue(tt_string_view name) const -> wxFontWeight
    {
        if (name.empty())
        {
            return wxFONTWEIGHT_NORMAL;
        }

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
            {
                return value;
            }
        }
        return wxFONTWEIGHT_NORMAL;
    }

    [[nodiscard]] static auto GetValue(wxFontWeight weight) -> std::string
    {
        switch (weight)
        {
            case wxFONTWEIGHT_THIN:
                return "wxFONTWEIGHT_THIN";
            case wxFONTWEIGHT_EXTRALIGHT:
                return "wxFONTWEIGHT_EXTRALIGHT";
            case wxFONTWEIGHT_LIGHT:
                return "wxFONTWEIGHT_LIGHT";
            case wxFONTWEIGHT_MEDIUM:
                return "wxFONTWEIGHT_MEDIUM";
            case wxFONTWEIGHT_SEMIBOLD:
                return "wxFONTWEIGHT_SEMIBOLD";
            case wxFONTWEIGHT_BOLD:
                return "wxFONTWEIGHT_BOLD";
            case wxFONTWEIGHT_EXTRABOLD:
                return "wxFONTWEIGHT_EXTRABOLD";
            case wxFONTWEIGHT_HEAVY:
                return "wxFONTWEIGHT_HEAVY";
            case wxFONTWEIGHT_EXTRAHEAVY:
                return "wxFONTWEIGHT_EXTRAHEAVY";

            case wxFONTWEIGHT_NORMAL:
            default:
                return "wxFONTWEIGHT_NORMAL";
        }
    }

    [[nodiscard]] auto GetName(wxFontWeight weight) const -> const std::string&
    {
        for (const auto& [key, value]: pairs)
        {
            if (weight == value)
            {
                return key;
            }
        }
        return tt_empty_cstr;
    }
};
extern const FontWeightPairs font_weight_pairs;

// Friendly name, wxFontStyle pair
struct FontStylePairs
{
    // clang-format off
    std::vector<std::pair<std::string, wxFontStyle>> pairs = {
        { "normal style", wxFONTSTYLE_NORMAL },
        { "italic", wxFONTSTYLE_ITALIC },
        { "slant", wxFONTSTYLE_SLANT },
    };
    // clang-format on

    auto GetPairs() const { return pairs; }

    [[nodiscard]] auto GetValue(tt_string_view name) const -> wxFontStyle
    {
        if (name.empty())
        {
            return wxFONTSTYLE_NORMAL;
        }

        for (const auto& [key, value]: pairs)
        {
            if (name.is_sameas(key))
            {
                return value;
            }
        }
        return wxFONTSTYLE_NORMAL;
    }

    [[nodiscard]] static auto GetValue(wxFontStyle style) -> std::string
    {
        switch (style)
        {
            case wxFONTSTYLE_ITALIC:
                return "wxFONTSTYLE_ITALIC";

            case wxFONTSTYLE_SLANT:
                return "wxFONTSTYLE_SLANT";

            default:
            case wxFONTSTYLE_NORMAL:
                return "wxFONTSTYLE_NORMAL";
        }
    }

    [[nodiscard]] auto GetName(wxFontStyle style) const -> const std::string&
    {
        for (const auto& [key, value]: pairs)
        {
            if (style == value)
            {
                return key;
            }
        }
        return tt_empty_cstr;
    }
};
extern const FontStylePairs font_style_pairs;
