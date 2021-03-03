/////////////////////////////////////////////////////////////////////////////
// Purpose:   Base widget generator class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/event.h>  // Event classes

#include "base_generator.h"

#include "../mainframe.h"             // MainFrame -- Main window frame
#include "../mockup/mockup_parent.h"  // Top-level MockUp Parent window

MockupParent* BaseGenerator::GetMockup()
{
    return wxGetFrame().GetMockup();
}

void BaseGenerator::OnLeftClick(wxMouseEvent& event)
{
    auto wxobject = event.GetEventObject();
    auto node = wxGetFrame().GetMockup()->GetNode(wxobject);

    if (wxGetFrame().GetSelectedNode() != node)
    {
        wxGetFrame().GetMockup()->SelectNode(wxobject);
    }
    event.Skip();
}
