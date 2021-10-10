/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;

#include "nodeinfo_base.h"

class NodeInfo : public NodeInfoBase
{
public:
    NodeInfo(wxWindow* parent, Node* cur_node = nullptr);
};
