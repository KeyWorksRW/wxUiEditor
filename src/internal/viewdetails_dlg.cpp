/////////////////////////////////////////////////////////////////////////////
// Purpose:   ViewDetails (node details) dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "viewdetails_dlg.h"  // auto-generated: generated/viewdetails_base.h and generated/viewdetails_base.cpp
#include <tuple>              // for std::ignore

// If this constructor is used, the caller must call Create(parent)
ViewDetails::ViewDetails() {}

ViewDetails::ViewDetails(wxWindow* parent)
{
    std::ignore = Create(parent);
}

void ViewDetails::OnInit(wxInitDialogEvent& event)
{
    event.Skip();  // transfer all validator data to their windows and update UI
}
