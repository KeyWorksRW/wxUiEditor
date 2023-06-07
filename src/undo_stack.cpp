/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "undo_stack.h"  // UndoStack

#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class

///////////////////////////////// UndoStack ////////////////////////////////////

void UndoStack::Push(UndoActionPtr ptr)
{
    if (!m_locked)
    {
        m_undo.push_back(ptr);
        m_redo.clear();
    }
    ptr->Change();
}

void UndoStack::Undo()
{
    if (m_undo.size())
    {
        auto command = m_undo.back();  // make a copy of the share_ptr to increase the reference count
        m_undo.pop_back();
        m_redo.push_back(command);
        command->Revert();
    }
}

void UndoStack::Redo()
{
    if (m_redo.size())
    {
        auto command = m_redo.back();  // make a copy of the share_ptr to increase the reference count
        m_redo.pop_back();
        m_undo.push_back(command);
        command->Change();
    }
}

wxString UndoStack::GetUndoString()
{
    wxString str;
    if (m_undo.size())
    {
        str = m_undo.back()->GetUndoString().wx_str();
    }
    return str;
}

wxString UndoStack::GetRedoString()
{
    wxString str;
    if (m_redo.size())
    {
        str = m_redo.back()->GetUndoString().wx_str();
    }
    return str;
}

///////////////////////////////// GroupUndoActions ////////////////////////////////////

GroupUndoActions::GroupUndoActions(const tt_string& undo_str, Node* sel_node) : UndoAction(undo_str.c_str())
{
    if (sel_node)
    {
        m_old_selected = wxGetFrame().GetSelectedNodePtr();
        m_selected_node = sel_node->GetSharedPtr();
    }
}

void GroupUndoActions::Change()
{
    for (auto& iter: m_actions)
    {
        iter->Change();
    }

    if (m_selected_node)
    {
        wxGetFrame().SelectNode(m_selected_node);
    }
}

void GroupUndoActions::Revert()
{
    for (auto& iter: m_actions)
    {
        iter->Revert();
    }

    if (m_old_selected)
    {
        wxGetFrame().SelectNode(m_old_selected);
    }
}
