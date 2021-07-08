/////////////////////////////////////////////////////////////////////////////
// Purpose:   Create and modify a node containing a wxGridBagSizer
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/gbsizer.h>  // wxGridBagSizer:  A sizer that can lay out items in a grid

#include "node_gridbag.h"  // GridBag class

#include "gridbag_item.h"  // GridBagItem -- Dialog for inserting an item into a wxGridBagSizer node
#include "mainapp.h"       // App -- Main application class
#include "mainframe.h"     // Main window frame
#include "node.h"          // Node class
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction

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
        ttlib::cstr undo_str;
        undo_str << "Append " << map_GenNames[new_node->gen_name()];
        wxGetFrame().PushUndoAction(std::make_shared<AppendGridBagAction>(new_node, gbsizer, undo_str));
        new_node->FixDuplicateName();
        wxGetFrame().FireCreatedEvent(new_node);
        wxGetFrame().SelectNode(new_node, true, true);
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

        ttlib::cstr undo_str;
        undo_str << "Append " << map_GenNames[new_node->gen_name()];
        wxGetFrame().PushUndoAction(std::make_shared<AppendGridBagAction>(new_node, gbsizer, undo_str, pos_append));
        new_node->FixDuplicateName();
        wxGetFrame().FireCreatedEvent(new_node);
        wxGetFrame().SelectNode(new_node, true, true);
        return true;
    }

    // If we get here, then either the row or the column must be inserted. That means any duplicate row/column needs to be
    // incremented, which has to be done recursively.

    ttlib::cstr undo_str;
    undo_str << "Insert " << map_GenNames[new_node->gen_name()];

    // Unlike a normal undo command, this one will simply make a copy of the current gbsizer and the current selection.
    auto undo_cmd = std::make_shared<GridBagAction>(gbsizer, undo_str);
    wxGetFrame().PushUndoAction(undo_cmd);

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
    undo_cmd->Update(gbsizer, new_node);
    wxGetFrame().FireCreatedEvent(new_node);
    wxGetFrame().SelectNode(new_node, true, true);

    return true;
}

size_t GridBag::IncrementRows(int row, Node* gbsizer)
{
    size_t result = tt::npos;
    for (size_t idx = 0; idx < gbsizer->GetChildCount(); ++idx)
    {
        if (gbsizer->GetChild(idx)->prop_as_int(prop_row) == row)
        {
            if (result == tt::npos)
                result = idx;
            gbsizer->GetChild(idx)->prop_set_value(prop_row, row + 1);
        }
    }

    ++row;
    if (row <= m_max_row)
        IncrementRows(row, gbsizer);

    return result;
}

size_t GridBag::IncrementColumns(int row, int column, Node* gbsizer)
{
    size_t result = tt::npos;
    for (size_t idx = 0; idx < gbsizer->GetChildCount(); ++idx)
    {
        if (gbsizer->GetChild(idx)->prop_as_int(prop_row) == row &&
            gbsizer->GetChild(idx)->prop_as_int(prop_column) == column)
        {
            if (result == tt::npos)
                result = idx;
            gbsizer->GetChild(idx)->prop_set_value(prop_column, column + 1);
        }
    }

    ++column;
    if (row <= m_max_column)
        IncrementColumns(row, column, gbsizer);

    return result;
}
