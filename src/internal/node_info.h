/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "generated/node_info_base.h"

class Node;  // node_info_base.h does not forward-declare this

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
