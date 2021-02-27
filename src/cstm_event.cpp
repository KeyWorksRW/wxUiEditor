/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Event handling
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "cstm_event.h"

#include "mainapp.h"     // MoveDirection -- Main application class
#include "mainframe.h"   // MainFrame -- Main window frame
#include "node_event.h"  // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "undo_cmds.h"   // Undoable command classes derived from UndoAction

wxDEFINE_EVENT(EVT_ProjectUpdated, CustomEvent);
wxDEFINE_EVENT(EVT_EventHandlerChanged, CustomEvent);

wxDEFINE_EVENT(EVT_NodeCreated, CustomEvent);
wxDEFINE_EVENT(EVT_NodeDeleted, CustomEvent);
wxDEFINE_EVENT(EVT_NodeSelected, CustomEvent);

wxDEFINE_EVENT(EVT_NodePropChange, CustomEvent);

void MainFrame::FireProjectLoadedEvent()
{
    ProjectLoaded();

    CustomEvent event(EVT_ProjectUpdated, wxGetApp().GetProjectPtr().get());
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(event);
    }
}

void MainFrame::FireSelectedEvent(Node* node)
{
    CustomEvent node_event(EVT_NodeSelected, node);
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(node_event);
    }
}

void MainFrame::FireCreatedEvent(Node* node)
{
    CustomEvent node_event(EVT_NodeCreated, node);
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(node_event);
    }
}

void MainFrame::FireDeletedEvent(Node* node)
{
    CustomEvent node_event(EVT_NodeDeleted, node);
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(node_event);
    }
}

void MainFrame::FirePropChangeEvent(NodeProperty* prop)
{
    CustomEvent node_event(EVT_NodePropChange, prop);
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(node_event);
    }
}

void MainFrame::FireProjectUpdatedEvent()
{
    CustomEvent event(EVT_ProjectUpdated, wxGetApp().GetProjectPtr().get());
    for (auto handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(event);
    }
}

void MainFrame::ChangeEventHandler(NodeEvent* event, const ttlib::cstr& value)
{
    if (event && value != event->get_value())
    {
        PushUndoAction(std::make_shared<ModifyEventAction>(event, value));
        CustomEvent evt_node(EVT_EventHandlerChanged, event);
        for (auto handler: m_custom_event_handlers)
        {
            handler->ProcessEvent(evt_node);
        }
    }
}
