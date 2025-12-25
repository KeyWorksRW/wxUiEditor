/////////////////////////////////////////////////////////////////////////////
// Purpose:   Maintain a undo and redo stack
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements a command pattern-based undo/redo system for wxUiEditor.
// UndoAction is an abstract base class requiring derived classes to implement Change()
// (apply/redo), Revert() (undo), and GetMemorySize(). GroupUndoActions allows multiple actions to
// be treated as a single undoable operation. UndoStack manages two vectors (undo/redo stacks) where
// Push() executes Change() and adds to undo stack, Undo() calls Revert() and moves to redo stack,
// and Redo() calls Change() and moves back to undo stack. The stack can be locked to execute
// actions without affecting undo/redo history. Actions may optionally store Node pointers and
// control UI selection events.

#pragma once

#include "wxue_namespace/wxue_string.h"

class Node;
class NodeProperty;
using NodeSharedPtr = std::shared_ptr<Node>;

class UndoAction
{
public:
    UndoAction(std::string_view undo_string = {})
    {
        if (undo_string.size())
        {
            m_undo_string = undo_string;
        }
    }

    virtual ~UndoAction() = default;

    // Called when pushed to the Undo stack and when Redo is called
    virtual auto Change() -> void = 0;

    // Called when Undo is requested
    virtual auto Revert() -> void = 0;

    // Size of the UndoAction object itself, plus any additional memory it allocates.
    [[nodiscard]] virtual auto GetMemorySize() -> size_t = 0;
    [[nodiscard]] virtual auto GetOldNode() -> NodeSharedPtr { return nullptr; }

    [[nodiscard]] auto GetUndoString() const -> const wxue::string& { return m_undo_string; }
    auto SetUndoString(std::string_view str) -> void { m_undo_string = str; }

    [[nodiscard]] auto wasUndoEventGenerated() -> bool { return m_UndoEventGenerated; }
    [[nodiscard]] auto wasRedoEventGenerated() -> bool { return m_RedoEventGenerated; }
    [[nodiscard]] auto wasUndoSelectEventGenerated() -> bool { return m_UndoSelectEventGenerated; }
    [[nodiscard]] auto wasRedoSelectEventGenerated() -> bool { return m_RedoSelectEventGenerated; }

    // Note that these will affect individual UndoActions added to GroupUndoActions, but will
    // not affect the GroupUndoActions class itself.

    auto AllowSelectEvent(bool allow) -> void { m_AllowSelectEvent = allow; }
    [[nodiscard]] auto isAllowedSelectEvent() -> bool { return m_AllowSelectEvent; }

    // This will only be valid if the action actually stored a node.
    [[nodiscard]] auto getNode() -> NodeSharedPtr { return m_node; }

    // This will only be valid if the action actually stored a property.
    [[nodiscard]] virtual auto GetProperty() -> NodeProperty* { return nullptr; }

protected:
    NodeSharedPtr m_node;

    wxue::string m_undo_string;

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
    GroupUndoActions(std::string_view undo_str, Node* sel_node = nullptr);

    // Called when pushed to the Undo stack and when Redo is called
    auto Change() -> void override;

    // Called when Undo is requested
    auto Revert() -> void override;

    auto Add(UndoActionPtr ptr) -> void { m_actions.push_back(ptr); }

    [[nodiscard]] auto GetMemorySize() -> size_t override;

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
    auto Push(UndoActionPtr ptr) -> void;

    // This will pop the last UndoAction, call UndoAction->Revert() and then push the
    // command on the redo stack
    auto Undo() -> void;

    // This will pop the last UndoAction from the redo stack, call UndoAction->Change()
    // and then push the command on the undo stack
    auto Redo() -> void;

    // When the stack is locked, Push() will call UndoAction->Change(), but will not change
    // the undo or redo stacks
    auto Lock() -> void { m_locked = true; }
    auto Unlock() -> void { m_locked = false; }
    [[nodiscard]] auto IsLocked() -> bool { return m_locked; }

    [[nodiscard]] auto IsUndoAvailable() -> bool { return m_undo.size(); }
    [[nodiscard]] auto IsRedoAvailable() -> bool { return m_redo.size(); }

    [[nodiscard]] auto GetUndoString() -> wxString;
    [[nodiscard]] auto GetRedoString() -> wxString;

    [[nodiscard]] auto size() -> size_t { return m_undo.size(); }

    [[nodiscard]] auto GetUndoVector() const -> const std::vector<UndoActionPtr>& { return m_undo; }
    [[nodiscard]] auto GetRedoVector() const -> const std::vector<UndoActionPtr>& { return m_redo; }

    auto clear() -> void
    {
        m_redo.clear();
        m_undo.clear();
    }

    // When undo is called, the command is popped and pushed onto the redo stack. So to get at the
    // last undo command, you have to get the last item in the redo stack. Redo works just the
    // opposite, pushing it's command to the last of the undo stack.

    [[nodiscard]] auto wasUndoEventGenerated() -> bool
    {
        return m_redo.back()->wasUndoEventGenerated();
    }
    [[nodiscard]] auto wasRedoEventGenerated() -> bool
    {
        return m_undo.back()->wasRedoEventGenerated();
    }
    [[nodiscard]] auto wasUndoSelectEventGenerated() -> bool
    {
        return m_redo.back()->wasUndoSelectEventGenerated();
    }
    [[nodiscard]] auto wasRedoSelectEventGenerated() -> bool
    {
        return m_undo.back()->wasRedoSelectEventGenerated();
    }

private:
    std::vector<UndoActionPtr> m_undo;
    std::vector<UndoActionPtr> m_redo;

    bool m_locked { false };
};
