/////////////////////////////////////////////////////////////////////////////
// Purpose:   Stores node types and allowable child count
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "node_types.h"

#include "gen_enums.h"     // Enumerations for generators
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class

using namespace child_count;
using namespace GenEnum;

int_t NodeType::GetAllowableChildren(GenType child_type, bool is_aui_parent) const
{
    if (auto result = m_map_children.find(child_type); result != m_map_children.end())
    {
        if (is_aui_parent && m_gen_type == type_form)
        {
            // wxAui forms do not use a top-level sizer

            if (child_type == type_sizer || child_type == type_gbsizer)
                return none;

            // Except for the "bar" types which both regular and aui forms only allow one of, all the other types can have
            // multiple children.
            if (result->second == none)
                return infinite;
        }
        return result->second;
    }
    return none;
}
