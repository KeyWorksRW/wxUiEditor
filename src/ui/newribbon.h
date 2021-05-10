/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "newribbon_base.h"

class NewRibbon : public NewRibbonBase
{
public:
    NewRibbon(wxWindow* parent = nullptr);

    // Checks current selected node to see if it accepts a wxRibbonBar as a child
    bool IsCreatable(bool notify_user = true);

    void CreateNode();
};
