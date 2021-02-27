/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class UndoAction
{
public:
    UndoAction(const char* ptr_undo_string = nullptr)
    {
        if (ptr_undo_string)
            m_undo_string = ptr_undo_string;
    }

    virtual ~UndoAction() = default;

    virtual void Change() = 0;
    virtual void Revert() = 0;

    ttlib::cstr GetUndoString() { return m_undo_string; }
    void SetUndoString(ttlib::cview str) { m_undo_string = str; }

private:
    ttlib::cstr m_undo_string;
};

using UndoActionPtr = std::shared_ptr<UndoAction>;

class UndoStack
{
public:
    // This will first call UndoAction->Change(), then clear the redo stack and push the
    // UndoAction onto the undo stack
    void Push(UndoActionPtr ptr);

    // This will pop the last UndoAction, call UndoAction->Revert() and then push the
    // command on the redo stack
    void Undo();

    // This will pop the last UndoAction from the redo stack, call UndoAction->Change()
    // and then push the command on the undo stack
    void Redo();

    // When the stack is locked, Push() will call UndoAction->Change(), but will not change
    // the undo or redo stacks
    void Lock() { m_locked = true; }
    void Unlock() { m_locked = false; }
    bool IsLocked() { return m_locked; }

    bool IsUndoAvailable() { return m_undo.size(); }
    bool IsRedoAvailable() { return m_redo.size(); }

    wxString GetUndoString();
    wxString GetRedoString();

    size_t size() { return m_undo.size(); }

    void clear()
    {
        m_redo.clear();
        m_undo.clear();
    }

private:
    std::vector<UndoActionPtr> m_undo;
    std::vector<UndoActionPtr> m_redo;

    bool m_locked { false };
};
