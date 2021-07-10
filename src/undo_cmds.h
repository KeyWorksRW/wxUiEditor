/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "node_classes.h"  // Forward defintions of Node classes
#include "undo_stack.h"    // UndoAction -- Maintain a undo and redo stack

class InsertNodeAction : public UndoAction
{
public:
    InsertNodeAction(Node* node, Node* parent, const ttlib::cstr& undo_str, int pos = -1);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_node;
    NodeSharedPtr m_old_selected;
    int m_pos;
    bool m_fix_duplicate_names { true };
};

class RemoveNodeAction : public UndoAction
{
public:
    RemoveNodeAction(Node* node, const ttlib::cstr& undo_str, bool AddToClipboard = false);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_node;
    NodeSharedPtr m_old_selected;
    size_t m_old_pos;
    bool m_AddToClipboard;
};

class ModifyPropertyAction : public UndoAction
{
public:
    ModifyPropertyAction(NodeProperty* prop, ttlib::cview value);
    ModifyPropertyAction(NodeProperty* prop, int value);
    void Change() override;
    void Revert() override;

private:
    NodeProperty* m_property;
    ttlib::cstr m_revert_value;
    ttlib::cstr m_change_value;
};

class ModifyEventAction : public UndoAction
{
public:
    ModifyEventAction(NodeEvent* event, ttlib::cview value);
    void Change() override;
    void Revert() override;

private:
    NodeEvent* m_event;
    ttlib::cstr m_revert_value;
    ttlib::cstr m_change_value;
};

class ChangePositionAction : public UndoAction
{
public:
    ChangePositionAction(Node* node, size_t position);
    void Change() override;
    void Revert() override;

    Node* GetParent() { return m_parent.get(); }
    Node* GetNode() { return m_node.get(); }

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_node;
    size_t m_change_pos;
    size_t m_revert_pos;
};

class ChangeParentAction : public UndoAction
{
public:
    ChangeParentAction(Node* node, Node* parent);
    void Change() override;
    void Revert() override;

    Node* GetOldParent() { return m_revert_parent.get(); }
    Node* GetNewParent() { return m_change_parent.get(); }
    Node* GetNode() { return m_node.get(); }

private:
    NodeSharedPtr m_node;
    NodeSharedPtr m_change_parent;
    NodeSharedPtr m_revert_parent;

    size_t m_revert_position;
    int m_revert_row;
    int m_revert_col;
};

class AppendGridBagAction : public UndoAction
{
public:
    AppendGridBagAction(Node* node, Node* parent, int pos = -1);
    void Change() override;
    void Revert() override;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_node;
    NodeSharedPtr m_old_selected;
    size_t m_old_pos;

    int m_pos;
    bool m_fix_duplicate_names { true };
};

// Use this when the entire wxGridBagSizer node needs to be saved.
class GridBagAction : public UndoAction
{
public:
    GridBagAction(Node* cur_gbsizer, const ttlib::cstr& undo_str);
    void Change() override;
    void Revert() override;

    // Call this after making all changes to the gbsizer children
    void Update(Node* cur_gbsizer, Node* selected);

private:
    NodeSharedPtr m_cur_gbsizer;
    NodeSharedPtr m_old_cur_gbsizer;  // Set when Update() is called
    NodeSharedPtr m_old_gbsizer;

    size_t m_idx_old_selected { 0 };
    size_t m_idx_cur_selected { 0 };
};
