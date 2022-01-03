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
    FontProperty(const wxFont& font);
    FontProperty(ttlib::cview font);
    FontProperty(NodeProperty* prop);
    FontProperty(wxVariant font);

    wxFont GetFont() const;

    void Convert(ttlib::cview font);
    wxString as_wxString() const;
    ttlib::cstr as_string() const;

    bool isDefGuiFont() const { return m_isDefGuiFont; }

    // True if default gui font but with a different point size
    bool isNonDefSize() const { return m_isDefGuiFont; }

    int GetPointSize() const { return std::lround(m_pointSize); }
    double GetFractionalPointSize() const { return m_pointSize; }

    wxFontStyle GetStyle() const
    {
        return m_flags & wxFONTFLAG_ITALIC ? wxFONTSTYLE_ITALIC :
               m_flags & wxFONTFLAG_SLANT  ? wxFONTSTYLE_SLANT :
                                             wxFONTSTYLE_NORMAL;
    }

    int GetWeight() const { return m_weight; }
    int GetNumericWeight() const { return m_weight; }

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

    // The following setters match the names in wxFontInfo so that code written for wxFontInfo or FontProperty can be used
    // interchangeably (at least for as setters and getters)

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

    FontProperty& Weight(int weight)
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
        if (style == wxFONTSTYLE_ITALIC)
            return Italic();

        if (style == wxFONTSTYLE_SLANT)
            return Slant();

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
    int m_weight { wxFONTWEIGHT_NORMAL };
    int m_flags { wxFONTFLAG_DEFAULT };

    bool m_isDefGuiFont { true };
    bool m_isNonDefSize { false };
};
