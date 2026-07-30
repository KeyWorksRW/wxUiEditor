/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Event handling
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "cstm_event.h"

#include "mainframe.h"        // MainFrame -- Main window frame
#include "node_event.h"       // NodeEventInfo -- NodeEvent and NodeEventInfo classes
#include "node_prop.h"        // NodeProperty class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction

wxDEFINE_EVENT(EVT_ProjectUpdated, CustomEvent);
wxDEFINE_EVENT(EVT_EventHandlerChanged, CustomEvent);
wxDEFINE_EVENT(EVT_ParentChanged, CustomEvent);
wxDEFINE_EVENT(EVT_PositionChanged, CustomEvent);

wxDEFINE_EVENT(EVT_NodeCreated, CustomEvent);
wxDEFINE_EVENT(EVT_NodeDeleted, CustomEvent);
wxDEFINE_EVENT(EVT_NodeSelected, CustomEvent);
wxDEFINE_EVENT(EVT_QueueSelect, CustomEvent);

wxDEFINE_EVENT(EVT_NodePropChange, CustomEvent);
wxDEFINE_EVENT(EVT_MultiPropChange, CustomEvent);

wxDEFINE_EVENT(EVT_GridBagAction, CustomEvent);

void MainFrame::FireProjectLoadedEvent()
{
    // The Project loaded event can be fired even if just the language is changed which can cause
    // the property grid to add or remove language categories. The easiest way to deal with the
    // potential redraw issue is simply to freeze the entire main window frame until all event
    // handlers have been processed.

    wxWindowUpdateLocker const freeze(this);

    ProjectLoaded();

    CustomEvent event(EVT_ProjectUpdated, Project.get_ProjectNode());
    for (auto* handler: m_custom_event_handlers)
    {
        handler->ProcessEvent(event);
    }

    // m_selected_node may be null -- handlers must check getNode() before dereference
    CustomEvent node_event(EVT_NodeSelected, m_selected_node.get());
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(node_event);
    }
}

void MainFrame::FireSelectedEvent(Node* node, size_t flags)
{
    CustomEvent node_event(EVT_NodeSelected, node);

    const auto handlers = m_custom_event_handlers;
    if (flags & evt_flags::queue_event)
    {
        for (auto* handler: handlers)
        {
            handler->QueueEvent(node_event.Clone());
        }
    }
    else
    {
        for (auto* handler: handlers)
        {
            handler->ProcessEvent(node_event);
        }
    }
}

void MainFrame::FireCreatedEvent(Node* node)
{
    CustomEvent node_event(EVT_NodeCreated, node);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(node_event);
    }
    UpdateWakaTime();
}

void MainFrame::FireDeletedEvent(Node* node)
{
    CustomEvent node_event(EVT_NodeDeleted, node);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(node_event);
    }
    UpdateWakaTime();
}

void MainFrame::FirePropChangeEvent(NodeProperty* prop)
{
    CustomEvent node_event(EVT_NodePropChange, prop);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(node_event);
    }
    UpdateWakaTime();
}

void MainFrame::FireMultiPropEvent(ModifyProperties* undo_cmd)
{
    CustomEvent node_event(EVT_MultiPropChange, undo_cmd);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(node_event);
    }
    UpdateWakaTime();
}

void MainFrame::FireProjectUpdatedEvent()
{
    CustomEvent event(EVT_ProjectUpdated, Project.get_ProjectNode());
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(event);
    }
}

void MainFrame::FireChangeEventHandler(NodeEvent* evt_node)
{
    CustomEvent event(EVT_EventHandlerChanged, evt_node);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(event);
    }
}

void MainFrame::FireParentChangedEvent(ChangeParentAction* undo_cmd)
{
    CustomEvent event(EVT_ParentChanged, undo_cmd);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(event);
    }
    UpdateWakaTime();
}

void MainFrame::FirePositionChangedEvent(ChangePositionAction* undo_cmd)
{
    CustomEvent event(EVT_PositionChanged, undo_cmd);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(event);
    }
    UpdateWakaTime();
}

void MainFrame::FireGridBagActionEvent(GridBagAction* undo_cmd)
{
    CustomEvent event(EVT_GridBagAction, undo_cmd);
    const auto handlers = m_custom_event_handlers;
    for (auto* handler: handlers)
    {
        handler->ProcessEvent(event);
    }
    UpdateWakaTime();
}

Node* CustomEvent::getNode()
{
    if (m_node)
    {
        return m_node;
    }
    if (m_property)
    {
        return m_property->getNode();
    }
    if (m_event)
    {
        return m_event->getNode();
    }

    return nullptr;
}
