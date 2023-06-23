/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class Node;
class NodeProperty;
using NodeSharedPtr = std::shared_ptr<Node>;


class UndoAction
{
public:
    UndoAction(tt_string_view undo_string = tt_empty_cstr)
    {
        if (undo_string.size())
        {
            m_undo_string = undo_string;
        }
    }

    virtual ~UndoAction() = default;

    // Called when pushed to the Undo stack and when Redo is called
    virtual void Change() = 0;

    // Called when Undo is requested
    virtual void Revert() = 0;

    // Size of the UndoAction object itself, plus any additional memory it allocates.
    virtual size_t GetMemorySize() = 0;

    tt_string GetUndoString() { return m_undo_string; }
    void SetUndoString(tt_string_view str) { m_undo_string = str; }

    bool wasUndoEventGenerated() { return m_UndoEventGenerated; }
    bool wasRedoEventGenerated() { return m_RedoEventGenerated; }
    bool wasUndoSelectEventGenerated() { return m_UndoSelectEventGenerated; }
    bool wasRedoSelectEventGenerated() { return m_RedoSelectEventGenerated; }

    // Note that these will affect individual UndoActions added to GroupUndoActions, but will
    // not affect the GroupUndoActions class itself.

    void AllowSelectEvent(bool allow) { m_AllowSelectEvent = allow; }
    bool isAllowedSelectEvent() { return m_AllowSelectEvent; }

    // This will only be valid if the action actually stored a node.
    NodeSharedPtr GetNode() { return m_node; }


    // This will only be valid if the action actually stored a property.
    virtual NodeProperty* GetProperty() { return nullptr; }

protected:
    NodeSharedPtr m_node;

    tt_string m_undo_string;

    bool m_UndoEventGenerated { false };
    bool m_RedoEventGenerated { false };
    bool m_UndoSelectEventGenerated { false };
    bool m_RedoSelectEventGenerated { false };

    bool m_AllowSelectEvent { true };
};

class Node;
using UndoActionPtr = std::shared_ptr<UndoAction>;
using NodeSharedPtr = std::shared_ptr<Node>;

// This class can be used when you want to group multiple UndoActions into a single UndoAction
// with a single undo string.
class GroupUndoActions : public UndoAction
{
public:
    // Specify sel_node if you want the current selection changed after all the UndoActions
    // have been called by Change() or Revert()
    GroupUndoActions(const tt_string& undo_str, Node* sel_node = nullptr);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

    void Add(UndoActionPtr ptr) { m_actions.push_back(ptr); }

    size_t GetMemorySize() override;

private:
    std::vector<UndoActionPtr> m_actions;
    NodeSharedPtr m_selected_node { nullptr };
    NodeSharedPtr m_old_selected { nullptr };
};

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

    const std::vector<UndoActionPtr>& GetUndoVector() const { return m_undo; }
    const std::vector<UndoActionPtr>& GetRedoVector() const { return m_redo; }

    void clear()
    {
        m_redo.clear();
        m_undo.clear();
    }

    // When undo is called, the command is popped and pushed onto the redo stack. So to get at the last undo command, you
    // have to get the last item in the redo stack. Redo works just the opposite, pushing it's command to the last of the
    // undo stack.

    bool wasUndoEventGenerated() { return m_redo.back()->wasUndoEventGenerated(); }
    bool wasRedoEventGenerated() { return m_undo.back()->wasRedoEventGenerated(); }
    bool wasUndoSelectEventGenerated() { return m_redo.back()->wasUndoSelectEventGenerated(); }
    bool wasRedoSelectEventGenerated() { return m_undo.back()->wasRedoSelectEventGenerated(); }

private:
    std::vector<UndoActionPtr> m_undo;
    std::vector<UndoActionPtr> m_redo;

    bool m_locked { false };
};
