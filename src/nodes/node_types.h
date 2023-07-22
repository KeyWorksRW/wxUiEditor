/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class for storing allowable child generator types
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

    void Create(GenType getGenType) { m_gen_type = getGenType; }

    GenType getGenType() const noexcept { return m_gen_type; }
    bool isType(GenType type) const noexcept { return (type == m_gen_type); }

    ptrdiff_t getAllowableChildren(GenType child_gen_type) const
    {
        if (auto result = m_map_children.find(child_gen_type); result != m_map_children.end())
            return result->second;
        else
            return 0;
    }

    void addChild(GenType getGenType, ptrdiff_t max_children) { m_map_children[getGenType] = max_children; }

private:
    GenType m_gen_type;

    std::map<GenType, ptrdiff_t> m_map_children;
};
