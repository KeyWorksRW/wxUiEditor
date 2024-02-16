/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>

#include <wx/gdicmn.h>

class NodeProperty;

inline constexpr std::array<const char*, 5> s_type_names = { "Art", "Embed", "SVG", "XPM" };

struct ImageProperties
{
public:
    tt_string type { s_type_names[1] };
    tt_string image;

    NodeProperty* node_property;

    void InitValues(tt_string_view value);
    tt_string CombineValues();
    wxString CombineDefaultSize();

    void SetWidth(int width) { m_size.x = width; }
    void SetHeight(int height) { m_size.y = height; }
    void SetSize(wxSize size) { m_size = size; }

    void SetAnimationType() { m_isAnimationType = true; }
    bool IsAnimationType() { return m_isAnimationType; }

private:
    bool m_isAnimationType { false };
    wxSize m_size { wxDefaultSize };
};
