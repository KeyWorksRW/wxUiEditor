/////////////////////////////////////////////////////////////////////////////
// Purpose:   ViewDetails (node details) dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include "viewdetails_dlg.h"  // auto-generated: generated/viewdetails_base.h and generated/viewdetails_base.cpp
#include <tuple>              // for std::ignore

// If this constructor is used, the caller must call Create(parent)
ViewDetails::ViewDetails() {}

ViewDetails::ViewDetails(wxWindow* parent)
{
    if (!Create(parent))
    {
        wxLogError("Failed to create ViewDetails dialog");
    }
}

void ViewDetails::OnInit(wxInitDialogEvent& event)
{
    // Enforce the default-ctor contract: Create(parent) must have been called.
    // Create(parent) sets the parent window, so a null parent means Create was never called.
    wxASSERT_MSG(GetParent() != nullptr, "ViewDetails requires Create(parent) before use");

    event.Skip();  // transfer all validator data to their windows and update UI
}
