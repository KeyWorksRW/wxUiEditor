/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "undo_stack.h"  // UndoStack

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
