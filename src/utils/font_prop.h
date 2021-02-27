/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/font.h>     // wxFontBase class: the interface of wxFont
#include <wx/variant.h>  // wxVariant class, container for any type

class FontProperty
{
public:
    FontProperty(const wxFont& font);
    FontProperty(ttlib::cview font);
    FontProperty(wxVariant font);

    wxFont GetFont() const;

    void Convert(ttlib::cview font);
    wxString as_wxString() const;
    ttlib::cstr as_string() const;

    int GetPointSize() const { return m_pointSize; }
    wxFontFamily GetFamily() const { return m_family; }
    int GetStyle() const { return m_style; }
    int GetWeight() const { return m_weight; }
    bool isUnderlined() const { return m_underlined; }
    const wxString& GetFaceName() const { return m_face; }

    void SetPointSize(int value) { m_pointSize = value; }
    void SetFamily(int value) { m_family = static_cast<wxFontFamily>(value); }
    void SetStyle(int value) { m_style = static_cast<wxFontStyle>(value); }
    void SetWeight(int value) { m_weight = static_cast<wxFontWeight>(value); }
    void SetUnderlined(bool value) { m_underlined = value; }
    void SetFacename(const wxString& value) { m_face = value; }

    operator wxFont() const { return GetFont(); }

private:
    // REVIEW: [KeyWorks - 12-15-2020] Starting with 3.1.2, pointsize can be a float
    int m_pointSize { -1 };

    wxFontFamily m_family { wxFONTFAMILY_DEFAULT };
    wxFontStyle m_style { wxFONTSTYLE_NORMAL };
    wxFontWeight m_weight { wxFONTWEIGHT_NORMAL };
    wxString m_face;
    wxFontEncoding m_encoding { wxFONTENCODING_DEFAULT };

    bool m_underlined { false };
    bool m_strikethrough { false };
    bool m_aliased { false };
};
