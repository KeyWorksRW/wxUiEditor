/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for event function
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "evt_string_prop.h"

#include "../nodes/node_event.h"    // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "../ui/eventhandlerdlg.h"  // Dialog for editing event handlers

EventStringProperty::EventStringProperty(const wxString& label, NodeEvent* event) :
    wxStringProperty(label, wxPG_LABEL, event->get_value()), m_event(event)
{
}

bool EventStringDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid, wxPGProperty* WXUNUSED(property))
{
    EventHandlerDlg dlg(propGrid->GetPanel(), m_event);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
