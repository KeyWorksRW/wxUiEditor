/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "undo_stack.h"  // UndoStack

#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class

///////////////////////////////// UndoStack ////////////////////////////////////

void UndoStack::Push(const UndoActionPtr& ptr)
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
    if (!m_undo.empty())
    {
        const UndoActionPtr command =
            m_undo.back();  // make a copy of the share_ptr to increase the reference count
        m_undo.pop_back();
        m_redo.push_back(command);
        command->Revert();
    }
}

void UndoStack::Redo()
{
    if (!m_redo.empty())
    {
        const UndoActionPtr command =
            m_redo.back();  // make a copy of the share_ptr to increase the reference count
        m_redo.pop_back();
        m_undo.push_back(command);
        command->Change();
    }
}

wxString UndoStack::GetUndoString()
{
    wxString str;
    if (!m_undo.empty())
    {
        str = m_undo.back()->GetUndoString().wx();
    }
    return str;
}

wxString UndoStack::GetRedoString()
{
    wxString str;
    if (!m_redo.empty())
    {
        str = m_redo.back()->GetUndoString().wx();
    }
    return str;
}

///////////////////////////////// GroupUndoActions ////////////////////////////////////

GroupUndoActions::GroupUndoActions(std::string_view undo_str, Node* sel_node) : UndoAction(undo_str)
{
    if (sel_node)
    {
        m_old_selected = wxGetFrame().getSelectedNodePtr();
        m_selected_node = sel_node->get_SharedPtr();
    }
}

void GroupUndoActions::Change()
{
    for (const auto& iter: m_actions)
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
    for (auto iter = m_actions.rbegin(); iter != m_actions.rend(); ++iter)
    {
        (*iter)->Revert();
    }

    if (m_old_selected)
    {
        wxGetFrame().SelectNode(m_old_selected);
    }
}

size_t GroupUndoActions::GetMemorySize()
{
    size_t total = sizeof(*this);
    for (const auto& iter: m_actions)
    {
        total += iter->GetMemorySize();
    }
    return total;
}
