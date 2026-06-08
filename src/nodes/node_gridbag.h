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
    [[nodiscard]] auto InsertNode(Node* gbsizer, Node* new_node) -> bool;

    [[nodiscard]] static auto MoveNode(Node* node, MoveDirection where, bool check_only) -> bool;
    static auto GridBagSort(Node* gridbag) -> void;

protected:
    size_t IncrementColumns(int row, int column, Node* gbsizer);
    size_t IncrementRows(int row, Node* gbsizer);
    auto Initialize() -> void;

    static auto MoveLeft(Node* node) -> void;
    static auto MoveRight(Node* node) -> void;
    static auto MoveUp(Node* node) -> void;
    static auto MoveDown(Node* node) -> void;

private:
    Node* m_gridbag;

    int m_max_column { -1 };
    int m_max_row { -1 };
};
