/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "eventhandlerdlg_base.h"

class NodeEvent;

class EventHandlerDlg : public EventHandlerDlgBase
{
public:
    EventHandlerDlg(wxWindow* parent, NodeEvent* event);
    const wxString& GetResults() { return m_value; }

protected:
    // Handlers for EventHandlerDlgBase events
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnUseFunction(wxCommandEvent& WXUNUSED(event)) override;
    void OnFunctionText(wxCommandEvent& WXUNUSED(event)) override;
    void OnUseLambda(wxCommandEvent& WXUNUSED(event)) override;
    void OnCapture(wxCommandEvent& WXUNUSED(event)) override;
    void OnIncludeEvent(wxCommandEvent& WXUNUSED(event)) override;

    wxString m_value;

private:
    NodeEvent* m_event;
};
