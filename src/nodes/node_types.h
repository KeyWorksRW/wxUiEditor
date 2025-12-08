/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for storing allowable child generator types
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements NodeType, defining parent-child relationship rules for component
// types in the node tree. Each NodeType has m_gen_type (the parent's GenType) and m_map_children
// (map<GenType, ptrdiff_t>) storing allowed child types with maximum child counts. Child count
// constants (child_count::none=0, infinite=-1, one=1, two=2) specify constraints like "wxBoxSizer
// accepts infinite type_widget children" or "wxDialog accepts one type_menubar child".
// get_AllowableChildren queries the map, returning 0 for disallowed types. NodeCreator populates
// these rules during initialization from XML generator definitions, and Node validation
// (is_ChildAllowed, AdoptChild) checks against these constraints before adding children. This
// centralized rule system enforces valid wxWidgets component hierarchies (e.g., preventing multiple
// menubars in a frame) across UI operations and import.

#pragma once

#include <map>

#include "gen_enums.h"  // Enumerations for generators
using namespace GenEnum;

namespace child_count
{
    constexpr ptrdiff_t none = 0;
    constexpr ptrdiff_t infinite = -1;
    constexpr ptrdiff_t one = 1;
    constexpr ptrdiff_t two = 2;
};  // namespace child_count

// Class for storing the type and amount of children the generator type can have.
class NodeType
{
public:
    NodeType() {}

    void Create(GenType get_GenType) { m_gen_type = get_GenType; }

    GenType get_GenType() const noexcept { return m_gen_type; }
    bool is_Type(GenType type) const noexcept { return (type == m_gen_type); }

    ptrdiff_t get_AllowableChildren(GenType child_gen_type) const
    {
        if (auto result = m_map_children.find(child_gen_type); result != m_map_children.end())
            return result->second;
        else
            return 0;
    }

    void AddChild(GenType get_GenType, ptrdiff_t max_children)
    {
        m_map_children[get_GenType] = max_children;
    }

private:
    GenType m_gen_type;

    std::map<GenType, ptrdiff_t> m_map_children;
};
