/////////////////////////////////////////////////////////////////////////////
// Purpose:   Create and modify a node containing a wxGridBagSizer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;

class GridBag
{
public:
    GridBag(Node* node_gridbag);

    auto GetMaxColumn() { return m_max_column; }
    auto GetMaxRow() { return m_max_row; }
    bool InsertNode(Node* gbsizer, Node* new_node);

protected:
    void Initialize();

private:
    Node* m_gridbag;

    int m_max_column { -1 };
    int m_max_row { -1 };
};
