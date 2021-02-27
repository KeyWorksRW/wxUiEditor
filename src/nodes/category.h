/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node property categories
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>
#include <vector>

class NodeCategory
{
public:
    NodeCategory(ttlib::cview name) { m_name = name; }

    const wxString& GetName() { return m_name; }
    ttlib::cstr getName() { return ttlib::cstr(m_name.wx_str()); }

    void AddProperty(const std::string& name) { m_properties.emplace_back(name); }

    const std::string& GetPropertyName(size_t index)
    {
        ASSERT(index < m_properties.size());
        return m_properties.at(index);
    }

    void AddEvent(const std::string& name) { m_events.emplace_back(name); }

    const std::string& GetEventName(size_t index)
    {
        ASSERT(index < m_events.size());
        return m_events.at(index);
    }

    NodeCategory& AddCategory(ttlib::cview name) { return m_categories.emplace_back(name); }
    std::vector<NodeCategory>& GetCategories() { return m_categories; }

    size_t GetPropertyCount() const { return m_properties.size(); }
    size_t GetEventCount() const { return m_events.size(); }
    size_t GetCategoryCount() const { return m_categories.size(); }

    std::vector<std::string>& GetProperties() { return m_properties; }
    std::vector<std::string>& GetEvents() { return m_events; }

private:
    // This has to be wxString instead of std::string because wxPropertyCategory and wxPropertyGridInterface require it
    wxString m_name;

    std::vector<std::string> m_properties;
    std::vector<std::string> m_events;
    std::vector<NodeCategory> m_categories;
};
