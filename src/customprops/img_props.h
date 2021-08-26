/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid image properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>

#include <wx/gdicmn.h>

class NodeProperty;

inline constexpr std::array<const char*, 4> s_type_names = {
    "Art",
    "Embed",
    "XPM",
    "Header",
};

struct ImageProperties
{
public:
    ttlib::cstr type { s_type_names[1] };
    ttlib::cstr image;

    NodeProperty* node_property;

    void InitValues(const char* value);
    ttlib::cstr CombineValues();
    wxString CombineScale();

    void SetWidth(int width) { m_size.x = width; }
    void SetHeight(int height) { m_size.y = height; }

    void SetAnimationType() { m_isAnimationType = true; }
    bool IsAnimationType() { return m_isAnimationType; }

private:
    bool m_isAnimationType { false };
    wxSize m_size { wxDefaultSize };
};
