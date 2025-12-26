/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements concrete UndoAction-derived classes for all undoable operations
// in wxUiEditor. Each class encapsulates a specific operation: InsertNodeAction/RemoveNodeAction
// (node tree modifications), ModifyPropertyAction/ModifyProperties (property changes),
// ModifyEventAction (event handler changes), ChangePositionAction/ChangeParentAction (node
// repositioning), ChangeSizerType/ChangeNodeType (component type changes),
// AppendGridBagAction/GridBagAction (wxGridBagSizer operations), and SortProjectAction (project
// sorting). Classes store both change and revert states (m_change_value/m_revert_value,
// m_change_parent/m_revert_parent, m_new_gen_sizer/m_old_node) enabling bidirectional execution.
// Change() applies the operation (called on push and redo), Revert() undoes it (called on undo).
// State tracking includes m_old_selected (restore selection), m_fire_created_event (event control),
// m_pos (insertion position). GetMemorySize() reports memory usage for history limits.
// GroupUndoActions (via ModifyProperties) batches operations into atomic undo units.

#pragma once

#include "gen_enums.h"   // Enumerations for generators
#include "undo_stack.h"  // UndoAction -- Maintain a undo and redo stack

class Node;
class NodeEvent;
class NodeProperty;

using NodeSharedPtr = std::shared_ptr<Node>;

// Specify node, parent node, undo string, and optional position
class InsertNodeAction : public UndoAction
{
public:
    InsertNodeAction(Node* node, Node* parent, std::string_view undo_str, int pos = -1);
    InsertNodeAction(const NodeSharedPtr node, const NodeSharedPtr parent,
                     std::string_view undo_str, int pos = -1);

    // Called when pushed to the Undo stack and when Redo is called
    auto Change() -> void override;

    // Called when Undo is requested
    auto Revert() -> void override;

    // Set this to true if you created the node without firing a created event.
    auto SetFireCreatedEvent(bool fire) -> void { m_fire_created_event = fire; }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

protected:
    auto Init(const NodeSharedPtr node, const NodeSharedPtr parent, std::string_view undo_str,
              int pos = -1) -> void;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_old_selected;
    int m_pos;
    bool m_fix_duplicate_names { true };
    bool m_fire_created_event { false };
};

// Specify node, undo string, and whether or not to add to the clipboard.
class RemoveNodeAction : public UndoAction
{
public:
    RemoveNodeAction(Node* node, std::string_view undo_str, bool AddToClipboard = false);
    RemoveNodeAction(const NodeSharedPtr node, std::string_view undo_str,
                     bool AddToClipboard = false);

    // Called when pushed to the Undo stack and when Redo is called
    auto Change() -> void override;

    // Called when Undo is requested
    auto Revert() -> void override;

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

protected:
    auto Init(const NodeSharedPtr node, std::string_view undo_str, bool AddToClipboard = false)
        -> void;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_old_selected;
    size_t m_old_pos;
    bool m_AddToClipboard;
};

// Specify property and value (string or int).
class ModifyPropertyAction : public UndoAction
{
public:
    ModifyPropertyAction(NodeProperty* prop, std::string_view value);
    ModifyPropertyAction(NodeProperty* prop, int value);
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetProperty() -> NodeProperty* override { return m_property; }

    // The +2 is to account for the trailing zero in each std::string value.
    [[nodiscard]] auto GetMemorySize() -> size_t override
    {
        return sizeof(*this) + m_revert_value.size() + m_change_value.size() + 2;
    }

private:
    NodeProperty* m_property;
    wxue::string m_revert_value;
    wxue::string m_change_value;
};

// Used to modify multiple properties as a single undo/redo command.
//
// Specify undo string, and whether or not to fire events.
class ModifyProperties : public UndoAction
{
public:
    ModifyProperties(std::string_view undo_string, bool fire_events = true);

    auto addProperty(NodeProperty* prop, std::string_view value) -> void;
    auto addProperty(NodeProperty* prop, int value) -> void;

    auto Change() -> void override;
    auto Revert() -> void override;

    struct MULTI_PROP
    {
        NodeProperty* property;
        // All properties are stored as a string, no matter what their original data type
        std::string revert_value;
        wxue::string change_value;
    };
    [[nodiscard]] auto& GetVector() { return m_properties; }

    [[nodiscard]] auto GetMemorySize() -> size_t override;

private:
    std::vector<MULTI_PROP> m_properties;
    bool m_fire_events { true };
};

// Specify event and value.
class ModifyEventAction : public UndoAction
{
public:
    ModifyEventAction(NodeEvent* event, std::string_view value);
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetMemorySize() -> size_t override
    {
        return sizeof(*this) + m_revert_value.size() + m_change_value.size();
    }

private:
    NodeEvent* m_event;
    std::string m_revert_value;
    std::string m_change_value;
};

// Specify node and position.
class ChangePositionAction : public UndoAction
{
public:
    ChangePositionAction(Node* node, size_t position);
    ChangePositionAction(const NodeSharedPtr node, size_t position);

    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto get_Parent() -> Node* { return m_parent.get(); }
    [[nodiscard]] auto getNode() -> Node* { return m_node.get(); }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

protected:
    auto Init(const NodeSharedPtr node, size_t position) -> void;

private:
    NodeSharedPtr m_parent;
    size_t m_change_pos;
    size_t m_revert_pos;
};

// Specify node and parent node.
class ChangeParentAction : public UndoAction
{
public:
    ChangeParentAction(Node* node, Node* parent, int pos = -1);
    ChangeParentAction(const NodeSharedPtr node, const NodeSharedPtr parent, int pos = -1);

    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetOldParent() -> Node* { return m_revert_parent.get(); }
    [[nodiscard]] auto GetNewParent() -> Node* { return m_change_parent.get(); }
    [[nodiscard]] auto getNode() -> Node* { return m_node.get(); }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

protected:
    auto Init(const NodeSharedPtr node, const NodeSharedPtr parent, int pos) -> void;

private:
    NodeSharedPtr m_change_parent;
    NodeSharedPtr m_revert_parent;

    size_t m_revert_position;
    int m_revert_row;
    int m_revert_col;
    int m_pos;
};

// Specify node and new sizer gen_ name.
class ChangeSizerType : public UndoAction
{
public:
    ChangeSizerType(Node* node, GenEnum::GenName new_sizer);
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetOldNode() -> NodeSharedPtr override { return m_old_node; }
    [[nodiscard]] auto getNode() -> Node* { return m_node.get(); }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

private:
    NodeSharedPtr m_old_node;
    NodeSharedPtr m_parent;
    GenEnum::GenName m_new_gen_sizer;
};

// Specify current node and new node gen_ name.
class ChangeNodeType : public UndoAction
{
public:
    ChangeNodeType(Node* node, GenEnum::GenName new_node);
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetOldNode() -> NodeSharedPtr override { return m_old_node; }
    [[nodiscard]] auto getNode() -> Node* { return m_node.get(); }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

private:
    NodeSharedPtr m_old_node;
    NodeSharedPtr m_parent;
    GenEnum::GenName m_new_gen_node;
};

// Specify node and parent node, and optional position
class AppendGridBagAction : public UndoAction
{
public:
    AppendGridBagAction(Node* node, Node* parent, int pos = -1);
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_old_selected;

    // REVIEW: [Randalphwa - 06-13-2022] m_old_pos can be -1, so it really should either be an int
    // or an int64_t.
    size_t m_old_pos;

    int m_pos;
    bool m_fix_duplicate_names { true };
};

// Use this when the entire wxGridBagSizer node needs to be saved. You *MUST* call Update()
// or the Navigation Panel will be frozen!
//
// Specify gridbag sizer node and undo string
class GridBagAction : public UndoAction
{
public:
    GridBagAction(Node* cur_gbsizer, std::string_view undo_str);
    auto Change() -> void override;
    auto Revert() -> void override;

    // Call this after making all changes to the gbsizer children
    auto Update() -> void;
    [[nodiscard]] auto GetOldSizerNode() const -> Node* { return m_old_gbsizer.get(); }
    [[nodiscard]] auto GetCurSizerNode() const -> Node* { return m_cur_gbsizer.get(); }

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }

private:
    NodeSharedPtr m_cur_gbsizer;
    NodeSharedPtr m_old_gbsizer;

    bool m_isReverted { false };
};

// This sorts all the children of the project based on their class_name property
class SortProjectAction : public UndoAction
{
public:
    SortProjectAction();
    auto Change() -> void override;
    auto Revert() -> void override;

    [[nodiscard]] auto GetMemorySize() -> size_t override { return sizeof(*this); }
    [[nodiscard]] auto GetOldNode() -> NodeSharedPtr override { return m_old_project; }

protected:
    auto SortFolder(Node* folder) -> void;

private:
    NodeSharedPtr m_old_project;
};
