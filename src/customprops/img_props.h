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
    "Embed",
    "Header",
    "Art",
    "XPM",
};

struct ImageProperties
{
public:
    ttlib::cstr type { s_type_names[0] };
    ttlib::cstr image;
    ttlib::cstr convert;
    wxSize size { wxDefaultSize };

    NodeProperty* node_property;

    void InitValues(const char* value);
    ttlib::cstr CombineValues();
};
