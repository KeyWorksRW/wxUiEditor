/////////////////////////////////////////////////////////////////////////////
// Purpose:   Create and modify a node containing a wxGridBagSizer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_classes.h"  // Forward defintions of Node classes

class GridBag
{
public:
    GridBag(Node* node_gridbag);


    auto GetMaxColumn() { return m_max_column; }
    auto GetMaxRow() { return m_max_row; }
    bool InsertNode(Node* gbsizer, Node* new_node);

    static bool MoveNode(Node* node, MoveDirection where, bool check_only);
    static void GridBagSort(Node* gridbag);

protected:
    size_t IncrementColumns(int row, int column, Node* gbsizer);
    size_t IncrementRows(int row, Node* gbsizer);
    void Initialize();

    static void MoveLeft(Node* node);
    static void MoveRight(Node* node);
    static void MoveUp(Node* node);
    static void MoveDown(Node* node);

private:
    Node* m_gridbag;

    int m_max_column { -1 };
    int m_max_row { -1 };
};
