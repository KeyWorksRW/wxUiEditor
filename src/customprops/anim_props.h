/////////////////////////////////////////////////////////////////////////////
// Purpose:   Handles property grid animation properties
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>

#include <wx/gdicmn.h>

#include "ttcstr.h"  // cstr -- std::string with additional methods

class NodeProperty;

struct AnimationProperties
{
public:
    ttlib::cstr image;
    ttlib::cstr convert;

    NodeProperty* node_property;

    void InitValues(const char* value);
    ttlib::cstr CombineValues();
};
