/////////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Event class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/event.h>

#include "node_classes.h"  // Forward defintions of Node classes

class CustomEvent : public wxEvent
{
public:
    CustomEvent(wxEventType commandType, Node* node) : wxEvent(0, commandType), m_node(node) {}
    CustomEvent(wxEventType commandType, NodeProperty* property) : wxEvent(0, commandType), m_property(property) {}
    CustomEvent(wxEventType commandType, NodeEvent* event) : wxEvent(0, commandType), m_event(event) {}

    Node* GetNode() { return m_node; }
    NodeProperty* GetNodeProperty() { return m_property; }
    NodeEvent* GetEventNode() { return m_event; }

    wxEvent* Clone() const override { return new CustomEvent(*this); }  // required to instantiate wxEvent class

private:
    Node* m_node { nullptr };
    NodeProperty* m_property { nullptr };
    NodeEvent* m_event { nullptr };
};

wxDECLARE_EVENT(EVT_ProjectUpdated, CustomEvent);
wxDECLARE_EVENT(EVT_EventHandlerChanged, CustomEvent);

wxDECLARE_EVENT(EVT_NodeCreated, CustomEvent);
wxDECLARE_EVENT(EVT_NodeDeleted, CustomEvent);
wxDECLARE_EVENT(EVT_NodeSelected, CustomEvent);

wxDECLARE_EVENT(EVT_NodePropChange, CustomEvent);
