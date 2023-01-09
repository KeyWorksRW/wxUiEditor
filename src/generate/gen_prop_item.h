/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid/Manager Item generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PropertyGridItemGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;

    ttlib::cstr GetHelpText(Node*) override;
    ttlib::cstr GetHelpURL(Node*) override;
};
