/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeEvent and NodeEventInfo classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements a two-class pattern separating immutable event metadata
// (NodeEventInfo) from user-specific event instances (NodeEvent). NodeEventInfo stores constant
// data shared across all uses of an event type: m_name (event name like "OnButtonClick"),
// m_event_class (wxWidgets class like "wxCommandEvent"), and m_help (description text). These are
// created once during initialization and owned by NodeDeclaration. NodeEvent represents a
// user-configured event binding with m_info (pointer to shared NodeEventInfo), m_node (owning
// Node*), and m_value (user's handler function name). This separation optimizes memory by sharing
// metadata while allowing each node to have unique handler names. NodeEvent provides convenience
// accessors (get_name, getNode) and size calculation (get_EventSize) for memory tracking.

#pragma once

class Node;

// This class is used to store fixed information about an event that never changes (it is
// created during app initialization).
class NodeEventInfo
{
public:
    NodeEventInfo(std::string_view name, std::string_view event_class, std::string_view help) :
        m_name(name), m_event_class(event_class), m_help(help)
    {
    }

    const wxue::string& get_name() const noexcept { return m_name; }
    const wxue::string& get_event_class() const noexcept { return m_event_class; }
    const wxue::string& get_help() const noexcept { return m_help; }

private:
    wxue::string m_name;
    wxue::string m_event_class;
    wxue::string m_help;
};

// This class is used to store event information specific to what the user has requsted (node
// containing the event, name of the event handler) along with a pointer to the fixed event
// information.
class NodeEvent
{
public:
    NodeEvent(const NodeEventInfo* info, Node* node) : m_info(info), m_node(node) {}

    void set_value(std::string_view value) { m_value = value; }
    const wxue::string& get_value() const noexcept { return m_value; }
    const wxue::string& get_name() const noexcept { return m_info->get_name(); }

    Node* getNode() const noexcept { return m_node; }
    const NodeEventInfo* get_EventInfo() const noexcept { return m_info; }

    // Currently only called in debug builds, but available for release builds should we need it
    size_t get_EventSize() const noexcept { return sizeof(*this) + (m_value.size() + 1); }

private:
    const NodeEventInfo* m_info;
    Node* m_node;

    wxue::string m_value;  // handler function name
};
