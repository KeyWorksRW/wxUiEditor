/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for storing node types and allowable child count
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

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
    NodeType() {}

    void Create(GenType gen_type)
    {
        m_gen_type = gen_type;
        m_name = map_GenTypes[gen_type];
    }

    const ttlib::cstr& get_name() const noexcept { return m_name; }
    GenType gen_type() const noexcept { return m_gen_type; }
    bool isType(GenType type) const noexcept { return (type == m_gen_type); }

    int_t GetAllowableChildren(GenType child_gen_type, bool is_aui_parent = false) const;

    int_t GetAllowableChildren(NodeType* child_type, bool is_aui_parent = false) const
    {
        return GetAllowableChildren(child_type->gen_type(), is_aui_parent);
    }

    void AddChild(GenType gen_type, int_t max_children) { m_map_children[gen_type] = max_children; }

private:
    // It's rare, but sometimes we need to check for a partial name such as "book" to match multiple types
    ttlib::cstr m_name;

    GenType m_gen_type;

    std::map<GenType, int_t> m_map_children;
};
