/////////////////////////////////////////////////////////////////////////////
// Purpose:   PropertyGrid/Manager Category generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PropertyGridCategoryGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;

    tt_string GetHelpText(Node*) override;
    tt_string GetHelpURL(Node*) override;
};
