/////////////////////////////////////////////////////////////////////////////
// Purpose:   NodeEvent and NodeEventInfo classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

    const tt_string& get_name() const noexcept { return m_name; }
    const tt_string& get_event_class() const noexcept { return m_event_class; }
    const tt_string& get_help() const noexcept { return m_help; }

private:
    tt_string m_name;
    tt_string m_event_class;
    tt_string m_help;
};

// This class is used to store event information specific to what the user has requsted (node
// containing the event, name of the event handler) along with a pointer to the fixed event
// information.
class NodeEvent
{
public:
    NodeEvent(const NodeEventInfo* info, Node* node) : m_info(info), m_node(node) {}

    void set_value(std::string_view value) { m_value = value; }
    const tt_string& get_value() const noexcept { return m_value; }
    const tt_string& get_name() const noexcept { return m_info->get_name(); }

    Node* getNode() const noexcept { return m_node; }
    const NodeEventInfo* getEventInfo() const noexcept { return m_info; }

    // Currently only called in debug builds, but available for release builds should we need it
    size_t getEventSize() const noexcept { return sizeof(*this) + (m_value.size() + 1); }

private:
    const NodeEventInfo* m_info;
    Node* m_node;

    tt_string m_value;  // handler function name
};
