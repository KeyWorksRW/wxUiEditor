/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "nodeinfo_base.h"

class Node;

class NodeInfo : public NodeInfoBase
{
public:
    NodeInfo();  // If you use this constructor, you must call Create(parent)
    NodeInfo(wxWindow* parent);

    void SetNode(Node* node) { m_node = node; }

protected:
    // Handlers for NodeInfoBase events
    void OnInit(wxInitDialogEvent& event) override;

private:
    Node* m_node { nullptr };
};
