/////////////////////////////////////////////////////////////////////////////
// Purpose:   ViewDetails (node details) dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "viewdetails_base.h"

class ViewDetails : public ViewDetails_base
{
public:
    ViewDetails();  // If you use this constructor, you must call Create(parent)
    ViewDetails(wxWindow* parent);

protected:
    // Handlers for ViewDetails_base events
    void OnInit(wxInitDialogEvent& event) override;
};
