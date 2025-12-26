/////////////////////////////////////////////////////////////////////////////
// Purpose:   Create and modify a node containing a wxGridBagSizer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements GridBag, a helper class for managing wxGridBagSizer node
// operations including insertion, movement, and layout calculations. The class wraps a
// wxGridBagSizer node (m_gridbag) and tracks grid dimensions (m_max_column, m_max_row) computed
// during Initialize(). InsertNode handles complex insertion logic by finding empty cells and
// adjusting row/column spans via IncrementColumns/IncrementRows when conflicts occur. Static
// methods MoveNode, MoveLeft, MoveRight, MoveUp, MoveDown modify child node positions by updating
// their prop_row/prop_column properties while respecting grid boundaries and span constraints.
// GridBagSort reorders children in memory to match visual grid layout (top-to-bottom,
// left-to-right). This class encapsulates GridBag- specific complexity separate from general Node
// tree operations, supporting both UI manipulation and undo/redo.

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
