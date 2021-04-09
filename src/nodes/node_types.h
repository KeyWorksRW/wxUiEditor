/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for storing node types and allowable child count
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include "enum_classes.h"  // Enumerations for nodes
using namespace NodeEnums;

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

    void Create(ClassType class_type)
    {
        m_class_type = class_type;
        m_name = map_ClassTypes[class_type];
    }

    const ttlib::cstr& get_name() const noexcept { return m_name; }
    ClassType get_type() const noexcept { return m_class_type; }

    int_t GetAllowableChildren(ClassType child_class_type, bool is_aui_parent = false) const;

    int_t GetAllowableChildren(NodeType* child_type, bool is_aui_parent = false) const
    {
        return GetAllowableChildren(child_type->get_type(), is_aui_parent);
    }

    void AddChild(ClassType class_type, int_t max_children) { m_map_children[class_type] = max_children; }

private:
    // It's rare, but sometimes we need to check for a partial name such as "book" to match multiple types
    ttlib::cstr m_name;

    ClassType m_class_type;

    std::map<ClassType, int_t> m_map_children;
};
