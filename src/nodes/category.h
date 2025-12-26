/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node property categories
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements NodeCategory, a hierarchical container for organizing node
// properties and events in the wxUiEditor property grid UI. Each category has a name (stored as
// wxString for wxPropertyGrid compatibility), vectors of property names (PropName enums), event
// names (std::string), and nested subcategories (std::vector<NodeCategory>). Categories are built
// during node initialization and queried during property grid population to group related
// properties (e.g., "Window Styles", "Common", "Events") with collapsible headers. Access methods
// (get_PropName, get_EventName, getCategoryCount) support iteration for UI display, while builder
// methods (addProperty, addEvent, addCategory) construct the hierarchy.

#pragma once

#include <vector>

#include "gen_enums.h"  // Enumerations for generators

using namespace GenEnum;

class NodeCategory
{
public:
    NodeCategory(std::string_view name) { m_name.assign(name.data(), name.size()); }

    const wxString& GetName() { return m_name; }
    wxue::string getName() { return wxue::string(m_name.utf8_string()); }

    void addProperty(PropName name) { m_prop_names.emplace_back(name); }

    PropName get_PropName(size_t index)
    {
        ASSERT(index < m_prop_names.size());
        return m_prop_names.at(index);
    }

    void addEvent(const std::string& name) { m_events.emplace_back(name); }

    const std::string& get_EventName(size_t index)
    {
        ASSERT(index < m_events.size());
        return m_events.at(index);
    }

    NodeCategory& addCategory(std::string_view name) { return m_categories.emplace_back(name); }
    std::vector<NodeCategory>& getCategories() { return m_categories; }

    size_t get_PropNameCount() const { return m_prop_names.size(); }
    size_t get_EventCount() const { return m_events.size(); }
    size_t getCategoryCount() const { return m_categories.size(); }

    std::vector<std::string>& get_Events() { return m_events; }

private:
    // This has to be wxString instead of std::string because wxPropertyCategory and
    // wxPropertyGridInterface require it
    wxString m_name;

    std::vector<PropName> m_prop_names;
    std::vector<std::string> m_events;
    std::vector<NodeCategory> m_categories;
};
