/////////////////////////////////////////////////////////////////////////////
// Purpose:   FontProperty class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2019-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "wx/font.h"  // wxFontBase class: the interface of wxFont

class wxVariant;

class FontProperty
{
public:
    FontProperty(const wxFont& font);
    FontProperty(ttlib::cview font);
    FontProperty(wxVariant font);

    ~FontProperty();

    wxFont GetFont() const;

    void Convert(ttlib::cview font);
    wxString as_wxString() const;
    ttlib::cstr as_string() const;

    bool isDefGuiFont() const { return m_isDefGuiFont; }

    // True if default gui font but with a different point size
    bool isNonDefSize() const { return m_isDefGuiFont; }

    // All constructors will have created m_info, so it is safe to use

    const wxFontInfo* GetFontInfo() const { return m_info; }

    int GetPointSize() const { return m_info->GetPointSize(); }
    double GetFractionalPointSize() const { return m_info->GetFractionalPointSize(); }
    wxFontFamily GetFamily() const { return m_info->GetFamily(); }
    int GetStyle() const { return m_info->GetStyle(); }
    int GetWeight() const { return m_info->GetWeight(); }
    const wxString& GetFaceName() const { return m_info->GetFaceName(); }

    bool isUnderlined() const { return m_info->IsUnderlined(); }
    bool isStrikeThrough() const { return m_info->IsStrikethrough(); }

    operator wxFont() const { return GetFont(); }

private:
    // We need to know the point size before we construct it.
    wxFontInfo* m_info { nullptr };
    bool m_isDefGuiFont { true };
    bool m_isNonDefSize { false };
};
