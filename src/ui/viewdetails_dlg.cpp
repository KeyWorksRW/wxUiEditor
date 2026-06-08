/////////////////////////////////////////////////////////////////////////////
// Purpose:   ViewDetails (node details) dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "viewdetails_dlg.h"  // auto-generated: ../wxui/viewdetails_base.h and ..//wxui/viewdetails_base.cpp

// If this constructor is used, the caller must call Create(parent)
ViewDetails::ViewDetails() {}

ViewDetails::ViewDetails(wxWindow* parent)
{
    (void) Create(parent);
}

void ViewDetails::OnInit(wxInitDialogEvent& event)
{
    event.Skip();  // transfer all validator data to their windows and update UI
}
