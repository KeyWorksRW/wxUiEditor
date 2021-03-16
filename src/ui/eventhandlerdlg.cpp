/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing event handlers
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "eventhandlerdlg.h"  // auto-generated: eventhandlerdlg_base.h and eventhandlerdlg_base.cpp

#include "../nodes/node_event.h"  // NodeEventInfo -- NodeEvent and NodeEventInfo classes

EventHandlerDlg::EventHandlerDlg(wxWindow* parent, NodeEvent* event) : EventHandlerDlgBase(parent), m_event(event)
{
    m_value = event->get_value().wx_str();
}

void EventHandlerDlg::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    // TODO: Implement OnInit
}

void EventHandlerDlg::OnUseFunction(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnUseFunction
}

void EventHandlerDlg::OnFunctionText(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnFunctionText
}

void EventHandlerDlg::OnUseLambda(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnUseLambda
}

void EventHandlerDlg::OnCapture(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnCapture
}

void EventHandlerDlg::OnIncludeEvent(wxCommandEvent& WXUNUSED(event))
{
    // TODO: Implement OnIncludeEvent
}
