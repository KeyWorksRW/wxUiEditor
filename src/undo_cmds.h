/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gen_enums.h"     // Enumerations for generators
#include "node_classes.h"  // Forward defintions of Node classes
#include "undo_stack.h"    // UndoAction -- Maintain a undo and redo stack

// Specify node, parent node, undo string, and optional position
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

// Specify node, undo string, and whether or not to add to the clipboard.
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

// Specify property and value (string or int).
class ModifyPropertyAction : public UndoAction
{
public:
    ModifyPropertyAction(NodeProperty* prop, ttlib::sview value);
    ModifyPropertyAction(NodeProperty* prop, int value);
    void Change() override;
    void Revert() override;

private:
    NodeProperty* m_property;
    ttlib::cstr m_revert_value;
    ttlib::cstr m_change_value;
};

// Used to modify multiple properties as a single undo/redo command.
//
// Specify undo string, and whether or not to fire events.
class ModifyProperties : public UndoAction
{
public:
    ModifyProperties(ttlib::sview undo_string, bool fire_events = true);

    void AddProperty(NodeProperty* prop, ttlib::sview value);
    void AddProperty(NodeProperty* prop, int value);

    void Change() override;
    void Revert() override;

    struct MULTI_PROP
    {
        NodeProperty* property;
        // All properties are stored as a string, no matter what their original data type
        ttlib::cstr revert_value;
        ttlib::cstr change_value;
    };
    auto& GetVector() { return m_properties; }

private:
    std::vector<MULTI_PROP> m_properties;
    bool m_fire_events { true };
};

// Specify event and value.
class ModifyEventAction : public UndoAction
{
public:
    ModifyEventAction(NodeEvent* event, ttlib::sview value);
    void Change() override;
    void Revert() override;

private:
    NodeEvent* m_event;
    ttlib::cstr m_revert_value;
    ttlib::cstr m_change_value;
};

// Specify node and position.
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

// Specify node and parent node.
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

// Specify node and new sizer gen_ name.
class ChangeSizerType : public UndoAction
{
public:
    ChangeSizerType(Node* node, GenEnum::GenName new_sizer);
    void Change() override;
    void Revert() override;

    Node* GetOldNode() { return m_old_node.get(); }
    Node* GetNode() { return m_node.get(); }

private:
    NodeSharedPtr m_node;
    NodeSharedPtr m_old_node;
    NodeSharedPtr m_parent;
    GenEnum::GenName m_new_gen_sizer;
};

// Specify current node and new node gen_ name.
class ChangeNodeType : public UndoAction
{
public:
    ChangeNodeType(Node* node, GenEnum::GenName new_node);
    void Change() override;
    void Revert() override;

    Node* GetOldNode() { return m_old_node.get(); }
    Node* GetNode() { return m_node.get(); }

private:
    NodeSharedPtr m_node;
    NodeSharedPtr m_old_node;
    NodeSharedPtr m_parent;
    GenEnum::GenName m_new_gen_node;
};

// Specify node and parent node, and optional position
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

    // REVIEW: [Randalphwa - 06-13-2022] m_old_pos can be -1, so it really should either be an int or an int64_t.
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
    GridBagAction(Node* cur_gbsizer, const ttlib::cstr& undo_str);
    void Change() override;
    void Revert() override;

    // Call this after making all changes to the gbsizer children
    void Update();
    Node* GetOldSizerNode() const { return m_old_gbsizer.get(); }
    Node* GetCurSizerNode() const { return m_cur_gbsizer.get(); }

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
    void Change() override;
    void Revert() override;

private:
    NodeSharedPtr m_old_project;
};
