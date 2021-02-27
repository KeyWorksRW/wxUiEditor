/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for storing node types and allowable child count
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <unordered_map>

namespace child_count
{
    constexpr int_t none = 0;
    constexpr int_t infinite = -1;
    constexpr int_t one = 1;
    constexpr int_t two = 2;
};  // namespace child_count

// Class for storing the type and amount of children the component type can have.
class NodeType
{
public:
    NodeType(ttlib::cview name) : m_name(name) {}

    const ttlib::cstr& get_name() const noexcept { return m_name; }

    int_t GetAllowableChildren(ttlib::cview child_name, bool is_aui_parent = false) const;

    int_t GetAllowableChildren(NodeType* child_type, bool is_aui_parent = false) const
    {
        return GetAllowableChildren(child_type->get_name(), is_aui_parent);
    }

    void AddChild(const char* name, int_t max_children);

private:
    ttlib::cstr m_name;

    struct AllowableChildren
    {
        AllowableChildren(int_t max) { max_children = max; }
        int_t max_children;
    };

    std::unordered_map<std::string, std::unique_ptr<AllowableChildren>> m_children;
};
