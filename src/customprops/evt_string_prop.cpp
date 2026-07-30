/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for event function
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "evt_string_prop.h"

#include "eventhandler_dlg.h"  // Dialog for editing event handlers

#include "../nodes/node_event.h"  // NodeEventInfo -- NodeEvent and NodeEventInfo classes

EventStringProperty::EventStringProperty(const wxString& label, NodeEvent* event) :
    wxStringProperty(label, wxPG_LABEL, event->get_value()),
    m_event(event)
{
}

bool EventStringDialogAdapter::DoShowDialog(wxPropertyGrid* propGrid,
                                            wxPGProperty* /* property unused */)
{
    EventHandlerDlg event_dialog(propGrid->GetPanel(), m_event);
    if (event_dialog.ShowModal() == wxID_OK)
    {
        SetValue(event_dialog.GetResults());
        return true;
    }

    return false;
}
