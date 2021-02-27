/////////////////////////////////////////////////////////////////////////////
// Purpose:   Load older version of wxUiEditor project
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_classes.h"  // Forward defintions of Node classes

namespace pugi
{
    class xml_document;
    class xml_node;
}  // namespace pugi

NodeSharedPtr LoadOldProject(pugi::xml_document& doc);

class OldProject
{
public:
    NodeSharedPtr CreateOldProjectNode(pugi::xml_node& xml_obj, Node* parent = nullptr, Node* sizeritem = nullptr);

private:
    // Indicates whether we have warned the user about a non-supported version
    bool m_not_supported_warning { false };
};
