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

auto UndoStack::Push(UndoActionPtr ptr) -> void
{
    if (!m_locked)
    {
        m_undo.push_back(ptr);
        m_redo.clear();
    }
    ptr->Change();
}

auto UndoStack::Undo() -> void
{
    if (m_undo.size())
    {
        const auto command =
            m_undo.back();  // make a copy of the share_ptr to increase the reference count
        m_undo.pop_back();
        m_redo.push_back(command);
        command->Revert();
    }
}

auto UndoStack::Redo() -> void
{
    if (m_redo.size())
    {
        const auto command =
            m_redo.back();  // make a copy of the share_ptr to increase the reference count
        m_redo.pop_back();
        m_undo.push_back(command);
        command->Change();
    }
}

auto UndoStack::GetUndoString() -> wxString
{
    wxString str;
    if (m_undo.size())
    {
        str = m_undo.back()->GetUndoString().make_wxString();
    }
    return str;
}

auto UndoStack::GetRedoString() -> wxString
{
    wxString str;
    if (m_redo.size())
    {
        str = m_redo.back()->GetUndoString().make_wxString();
    }
    return str;
}

///////////////////////////////// GroupUndoActions ////////////////////////////////////

GroupUndoActions::GroupUndoActions(const tt_string& undo_str, Node* sel_node) :
    UndoAction(undo_str.c_str())
{
    if (sel_node)
    {
        m_old_selected = wxGetFrame().getSelectedNodePtr();
        m_selected_node = sel_node->get_SharedPtr();
    }
}

auto GroupUndoActions::Change() -> void
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

auto GroupUndoActions::Revert() -> void
{
    for (const auto& iter: m_actions)
    {
        iter->Revert();
    }

    if (m_old_selected)
    {
        wxGetFrame().SelectNode(m_old_selected);
    }
}

auto GroupUndoActions::GetMemorySize() -> size_t
{
    size_t total = sizeof(*this);
    for (const auto& iter: m_actions)
    {
        total += iter->GetMemorySize();
    }
    return total;
}
