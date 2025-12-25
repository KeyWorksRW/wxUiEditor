/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>

#include <wx/gdicmn.h>

#include "wxue_namespace/wxue_string.h"  // wxue::string

class NodeProperty;

inline constexpr auto s_type_names = std::to_array<const char*>({ "Embed", "SVG", "Art", "XPM" });

struct ImageProperties
{
public:
    wxue::string type { s_type_names[0] };
    wxue::string image;

    NodeProperty* node_property;

    void InitValues(wxue::string_view value);
    wxue::string CombineValues();
    wxString CombineDefaultSize();

    void SetWidth(int width) { m_size.x = width; }
    void SetHeight(int height) { m_size.y = height; }
    void SetSize(wxSize size) { m_size = size; }
    wxSize GetSize() { return m_size; }

    void SetAnimationType() { m_isAnimationType = true; }
    bool IsAnimationType() { return m_isAnimationType; }

private:
    bool m_isAnimationType { false };
    wxSize m_size { wxDefaultSize };
    wxSize m_def_art_size { wxDefaultSize };
};
