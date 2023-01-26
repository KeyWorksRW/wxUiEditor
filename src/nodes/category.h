/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node property categories
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdexcept>
#include <vector>

#include "gen_enums.h"  // Enumerations for generators

using namespace GenEnum;

class NodeCategory
{
public:
    NodeCategory(std::string_view name) { m_name.assign(name.data(), name.size()); }

    const wxString& GetName() { return m_name; }
    tt_string getName() { return tt_string(m_name.wx_str()); }

    void AddProperty(PropName name) { m_prop_names.emplace_back(name); }

    PropName GetPropName(size_t index)
    {
        ASSERT(index < m_prop_names.size());
        return m_prop_names.at(index);
    }

    void AddEvent(const std::string& name) { m_events.emplace_back(name); }

    const std::string& GetEventName(size_t index)
    {
        ASSERT(index < m_events.size());
        return m_events.at(index);
    }

    NodeCategory& AddCategory(std::string_view name) { return m_categories.emplace_back(name); }
    std::vector<NodeCategory>& GetCategories() { return m_categories; }

    size_t GetPropNameCount() const { return m_prop_names.size(); }
    size_t GetEventCount() const { return m_events.size(); }
    size_t GetCategoryCount() const { return m_categories.size(); }

    std::vector<std::string>& GetEvents() { return m_events; }

private:
    // This has to be wxString instead of std::string because wxPropertyCategory and wxPropertyGridInterface require it
    wxString m_name;

    std::vector<PropName> m_prop_names;
    std::vector<std::string> m_events;
    std::vector<NodeCategory> m_categories;
};
