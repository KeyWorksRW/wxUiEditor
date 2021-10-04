/////////////////////////////////////////////////////////////////////////////
// Purpose:   Create and modify a node containing a wxGridBagSizer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/gbsizer.h>  // wxGridBagSizer:  A sizer that can lay out items in a grid

#include "node_gridbag.h"  // GridBag class

#include "gridbag_item.h"  // GridBagItem -- Dialog for inserting an item into a wxGridBagSizer node
#include "mainapp.h"       // App -- Main application class
#include "mainframe.h"     // Main window frame
#include "node.h"          // Node class
#include "undo_cmds.h"     // Undoable command classes derived from UndoAction

GridBag::GridBag(Node* node_gridbag) : m_gridbag(node_gridbag)
{
    Initialize();
}

void GridBag::Initialize()
{
    if (m_gridbag->GetChildCount())
    {
        m_max_column = 0;
        m_max_row = 0;
        for (size_t idx = 0; idx < m_gridbag->GetChildCount(); ++idx)
        {
            auto col = m_gridbag->GetChild(idx)->prop_as_int(prop_column);
            auto col_span = m_gridbag->GetChild(idx)->prop_as_int(prop_colspan);
            if (col_span > 1)
                col += (col_span - 1);
            if (col > m_max_column)
                m_max_column = col;

            auto row = m_gridbag->GetChild(idx)->prop_as_int(prop_row);
            auto row_span = m_gridbag->GetChild(idx)->prop_as_int(prop_rowspan);
            if (row_span > 1)
                row += (row_span - 1);
            if (row > m_max_row)
                m_max_row = row;
        }
    }
}

bool GridBag::InsertNode(Node* gbsizer, Node* new_node)
{
    GridBagItem dlg(wxGetApp().GetMainFrame());
    if (dlg.ShowModal() != wxID_OK)
        return false;

    new_node->prop_set_value(prop_column, dlg.GetColumn());
    new_node->prop_set_value(prop_colspan, dlg.GetColumnSpan());
    new_node->prop_set_value(prop_row, dlg.GetRow());
    new_node->prop_set_value(prop_rowspan, dlg.GetRowSpan());

    if (dlg.GetRow() > m_max_row)
    {
        wxGetFrame().PushUndoAction(std::make_shared<AppendGridBagAction>(new_node, gbsizer));
        return true;
    }

    if (dlg.GetAction() == GridBagItem::action_append)  // append the column
    {
        int_t pos_append = 0;
        int last_column = -1;

        // Both rows and columns can be in any random child position, so each node must be examined to find the last
        // column of the row we want to append to.
        for (size_t pos = 0; pos < gbsizer->GetChildCount(); ++pos)
        {
            if (gbsizer->GetChild(pos)->prop_as_int(prop_row) == dlg.GetRow())
            {
                if (gbsizer->GetChild(pos)->prop_as_int(prop_column) < dlg.GetColumn())
                {
                    if (gbsizer->GetChild(pos)->prop_as_int(prop_column) > last_column)
                    {
                        pos_append = static_cast<int_t>(pos);
                        last_column = gbsizer->GetChild(pos)->prop_as_int(prop_column);
                    }
                }
            }
        }

        // We need to add this after the last column found.
        ++pos_append;
        if (pos_append >= static_cast<int_t>(gbsizer->GetChildCount()))
            pos_append = -1;  // Append the child at the very end

        wxGetFrame().PushUndoAction(std::make_shared<AppendGridBagAction>(new_node, gbsizer, pos_append));
        return true;
    }

    // If we get here, then either the row or the column must be inserted. That means any duplicate row/column needs to be
    // incremented, which has to be done recursively.

    ttlib::cstr undo_str;
    undo_str << "Insert " << map_GenNames[new_node->gen_name()];

    // Unlike a normal undo command, this one will simply make a copy of the current gbsizer and the current selection.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

    // Inserting means we have to change multiple columns or multiple rows, and to do that we need the children correctly
    // sorted.

    GridBagSort(gbsizer);

    size_t insert_pos = 0;
    if (dlg.GetAction() == GridBagItem::action_insert_row)
    {
        insert_pos = IncrementRows(dlg.GetRow(), gbsizer);
    }
    else
    {
        insert_pos = IncrementColumns(dlg.GetRow(), dlg.GetColumn(), gbsizer);
    }

    gbsizer->AddChild(insert_pos, new_node);
    new_node->SetParent(gbsizer);
    undo_cmd->Update();
    wxGetFrame().FireGridBagActionEvent(undo_cmd.get());
    wxGetFrame().SelectNode(new_node, true, true);

    return true;
}

size_t GridBag::IncrementRows(int row, Node* gbsizer)
{
    size_t insert_position = tt::npos;
    for (size_t idx = 0; idx < gbsizer->GetChildCount(); ++idx)
    {
        if (gbsizer->GetChild(idx)->prop_as_int(prop_row) == row)
        {
            insert_position = idx;
            while (idx < gbsizer->GetChildCount())
            {
                gbsizer->GetChild(idx)->prop_set_value(prop_row, gbsizer->GetChild(idx)->prop_as_int(prop_row) + 1);
                ++idx;
            }
            break;
        }
    }

    return insert_position;
}

size_t GridBag::IncrementColumns(int row, int column, Node* gbsizer)
{
    size_t insert_position = tt::npos;
    for (size_t idx = 0; idx < gbsizer->GetChildCount(); ++idx)
    {
        if (gbsizer->GetChild(idx)->prop_as_int(prop_row) == row &&
            gbsizer->GetChild(idx)->prop_as_int(prop_column) == column)
        {
            insert_position = idx;
            while (idx < gbsizer->GetChildCount())
            {
                if (gbsizer->GetChild(idx)->prop_as_int(prop_row) != row)
                    break;
                gbsizer->GetChild(idx)->prop_set_value(prop_column, gbsizer->GetChild(idx)->prop_as_int(prop_column) + 1);
                ++idx;
            }
            break;
        }
    }

    return insert_position;
}

static bool CompareRowNodes(NodeSharedPtr a, NodeSharedPtr b)
{
    return (a->prop_as_int(prop_row) < b->prop_as_int(prop_row));
}

static bool CompareColumnNodes(NodeSharedPtr a, NodeSharedPtr b)
{
    return (a->prop_as_int(prop_column) < b->prop_as_int(prop_column));
}

void GridBag::GridBagSort(Node* gridbag)
{
    if (!gridbag->GetChildCount())
        return;  // no children, so nothing to do

    auto& grid_vector = gridbag->GetChildNodePtrs();

    // First sort the rows
    std::sort(grid_vector.begin(), grid_vector.end(), CompareRowNodes);

    // Now sort the columns within each row
    for (size_t idx = 0; idx < grid_vector.size() - 1;)
    {
        auto row = grid_vector[idx]->prop_as_int(prop_row);
        auto end = idx + 1;
        while (grid_vector[end]->prop_as_int(prop_row) == row)
        {
            ++end;
            if (end >= grid_vector.size())
                break;
        }

        std::sort(grid_vector.begin() + idx, grid_vector.begin() + end, CompareColumnNodes);

        idx = end;
    }
}

static void SwapNodes(Node* gbSizer, size_t first_pos, size_t second_pos)
{
    auto& vector = gbSizer->GetChildNodePtrs();
    auto temp = std::move(vector[first_pos]);
    vector[first_pos] = std::move(vector[second_pos]);
    vector[second_pos] = std::move(temp);
}

bool GridBag::MoveNode(Node* node, MoveDirection where, bool check_only)
{
    // This function is completely reliant on the children of the wxGridBagSizer being sorted. That means unless we are just
    // doing a check or know that no action can be taken, then we always resort the entire gridbagsizer.

    ASSERT(node->GetParent()->isGen(gen_wxGridBagSizer));

    if (where == MoveDirection::Left)
    {
        if (check_only || node->prop_as_int(prop_column) == 0)
        {
            return (node->prop_as_int(prop_column) > 0);
        }

        MoveLeft(node);
    }
    else if (where == MoveDirection::Right)
    {
        // Unless we decide to enforce a limit, the user can always increase the column number
        if (check_only)
            return true;

        MoveRight(node);
    }
    else if (where == MoveDirection::Up)
    {
        if (check_only || node->prop_as_int(prop_row) == 0)
        {
            return (node->prop_as_int(prop_row) > 0);
        }

        MoveUp(node);
    }
    else if (where == MoveDirection::Down)
    {
        // Unless we decide to enforce a limit, the user can always increase the row
        if (check_only)
            return true;

        MoveDown(node);
    }

    return false;
}

void GridBag::MoveLeft(Node* node)
{
    auto gbsizer = node->GetParent();
    ttlib::cstr undo_str;
    undo_str << "Change column of " << map_GenNames[node->gen_name()];

    // Unlike a normal undo command, this one will make a copy of the current gbsizer rather than the current node.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

    GridBagSort(gbsizer);

    auto cur_position = gbsizer->GetChildPosition(node);
    auto cur_row = node->prop_as_int(prop_row);
    auto cur_column = node->prop_as_int(prop_column);

    auto previous_node = gbsizer->GetChild(cur_position - 1);
    bool isColumnChangeOnly { false };
    if (previous_node->prop_as_int(prop_row) != cur_row)
    {
        isColumnChangeOnly = true;
    }
    else
    {
        auto previous_column = previous_node->prop_as_int(prop_column) + (previous_node->prop_as_int(prop_colspan) - 1);
        if (cur_column - 1 > previous_column)
            isColumnChangeOnly = true;
    }

    if (isColumnChangeOnly)
    {
        node->prop_set_value(prop_column, cur_column - 1);
    }
    else
    {
        node->prop_set_value(prop_column, previous_node->prop_as_int(prop_column));
        previous_node->prop_set_value(prop_column, cur_column);

        SwapNodes(gbsizer, cur_position - 1, cur_position);
    }

    // This needs to be called once the gbsizer has been modified
    undo_cmd->Update();
    wxGetFrame().FireGridBagActionEvent(undo_cmd.get());
    wxGetFrame().SelectNode(node, true, true);
}

void GridBag::MoveRight(Node* node)
{
    auto gbsizer = node->GetParent();

    ttlib::cstr undo_str;
    undo_str << "Change column of " << map_GenNames[node->gen_name()];
    // Unlike a normal undo command, this one will make a copy of the current gbsizer rather than the current node.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

    GridBagSort(gbsizer);

    auto cur_position = gbsizer->GetChildPosition(node);
    auto cur_row = node->prop_as_int(prop_row);
    auto cur_column = node->prop_as_int(prop_column);

    auto next_node = (cur_position + 1 < gbsizer->GetChildCount()) ? gbsizer->GetChild(cur_position + 1) : nullptr;

    bool isColumnChangeOnly { false };
    if (!next_node || next_node->prop_as_int(prop_row) != cur_row)
    {
        isColumnChangeOnly = true;
    }
    else if (cur_column + node->prop_as_int(prop_colspan) < next_node->prop_as_int(prop_column))
    {
        isColumnChangeOnly = true;
    }

    if (isColumnChangeOnly)
    {
        node->prop_set_value(prop_column, cur_column + 1);
    }
    else
    {
        // prop_colspan is always at least 1
        node->prop_set_value(prop_column, cur_column + next_node->prop_as_int(prop_colspan));
        next_node->prop_set_value(prop_column, cur_column);

        SwapNodes(gbsizer, cur_position + 1, cur_position);
    }

    undo_cmd->Update();
    wxGetFrame().FireGridBagActionEvent(undo_cmd.get());
    wxGetFrame().SelectNode(node, true, true);
}

void GridBag::MoveUp(Node* node)
{
    auto gbsizer = node->GetParent();

    ttlib::cstr undo_str;
    undo_str << "Change row";

    // Unlike a normal undo command, this one will make a copy of the current gbsizer rather than the current node.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

    GridBagSort(gbsizer);

    auto cur_row = node->prop_as_int(prop_row);
    auto begin_position = gbsizer->GetChildPosition(node);
    while (begin_position > 0)
    {
        if (gbsizer->GetChild(begin_position - 1)->prop_as_int(prop_row) == cur_row)
            --begin_position;
        else
            break;
    }

    auto end_position = begin_position + 1;
    while (end_position < gbsizer->GetChildCount())
    {
        if (gbsizer->GetChild(end_position)->prop_as_int(prop_row) == cur_row)
            ++end_position;
        else
            break;
    }

    if (begin_position == 0)
    {
        auto new_row = cur_row - 1;
        while (begin_position < end_position)
        {
            gbsizer->GetChild(begin_position)->prop_set_value(prop_row, new_row);
            ++begin_position;
        }
        undo_cmd->Update();
        wxGetFrame().SelectNode(node, true, true);
        return;
    }

    auto end_swap = begin_position - 1;
    auto new_row = gbsizer->GetChild(end_swap)->prop_as_int(prop_row);
    while (begin_position < end_position)
    {
        gbsizer->GetChild(begin_position)->prop_set_value(prop_row, new_row);
        ++begin_position;
    }

    for (;;)
    {
        gbsizer->GetChild(end_swap)->prop_set_value(prop_row, cur_row);
        --end_swap;
        if (gbsizer->GetChild(end_swap)->prop_as_int(prop_row) != new_row)
            break;
        else if (end_swap == 0)
        {
            gbsizer->GetChild(end_swap)->prop_set_value(prop_row, cur_row);
            break;
        }
    }

    // We've changed the row number for at least two items, and possible several more, so resort
    GridBagSort(gbsizer);

    undo_cmd->Update();
    wxGetFrame().FireGridBagActionEvent(undo_cmd.get());
    wxGetFrame().SelectNode(node, true, true);
}

void GridBag::MoveDown(Node* node)
{
    auto gbsizer = node->GetParent();

    ttlib::cstr undo_str;
    undo_str << "Change row";

    // Unlike a normal undo command, this one will make a copy of the current gbsizer rather than the current node.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

    GridBagSort(gbsizer);

    auto cur_row = node->prop_as_int(prop_row);
    auto begin_position = gbsizer->GetChildPosition(node);
    while (begin_position > 0)
    {
        if (gbsizer->GetChild(begin_position - 1)->prop_as_int(prop_row) == cur_row)
            --begin_position;
        else
            break;
    }

    auto end_position = begin_position + 1;
    while (end_position < gbsizer->GetChildCount())
    {
        if (gbsizer->GetChild(end_position)->prop_as_int(prop_row) == cur_row)
            ++end_position;
        else
            break;
    }

    if (end_position == gbsizer->GetChildCount())
    {
        auto new_row = cur_row + 1;
        while (begin_position < end_position)
        {
            gbsizer->GetChild(begin_position)->prop_set_value(prop_row, new_row);
            ++begin_position;
        }
        undo_cmd->Update();
        wxGetFrame().SelectNode(node, true, true);
        return;
    }

    auto end_swap = end_position;
    auto new_row = gbsizer->GetChild(end_swap)->prop_as_int(prop_row);
    while (begin_position < end_position)
    {
        gbsizer->GetChild(begin_position)->prop_set_value(prop_row, new_row);
        ++begin_position;
    }

    for (;;)
    {
        gbsizer->GetChild(end_swap)->prop_set_value(prop_row, cur_row);
        ++end_swap;
        if (end_swap == gbsizer->GetChildCount() || gbsizer->GetChild(end_swap)->prop_as_int(prop_row) != new_row)
            break;
    }

    // We've changed the row number for at least two items, and possible several more, so resort
    GridBagSort(gbsizer);

    undo_cmd->Update();
    wxGetFrame().FireGridBagActionEvent(undo_cmd.get());
    wxGetFrame().SelectNode(node, true, true);
}
