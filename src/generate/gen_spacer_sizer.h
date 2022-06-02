/////////////////////////////////////////////////////////////////////////////
// Purpose:   Add space to sizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// This class is needed to get it to display on the Sizers toolbar, but it's not an actual component
class SpacerGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
};
