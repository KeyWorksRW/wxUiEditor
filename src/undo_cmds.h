/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
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
    InsertNodeAction(Node* node, Node* parent, const tt_string& undo_str, int pos = -1);
    InsertNodeAction(const NodeSharedPtr node, const NodeSharedPtr parent, tt_string_view undo_str, int pos = -1);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

    // Set this to true if you created the node without firing a created event.
    void SetFireCreatedEvent(bool fire) { m_fire_created_event = fire; }

    size_t GetMemorySize() override { return sizeof(*this); }

protected:
    void Init(const NodeSharedPtr node, const NodeSharedPtr parent, tt_string_view undo_str, int pos = -1);

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
    RemoveNodeAction(Node* node, const tt_string& undo_str, bool AddToClipboard = false);
    RemoveNodeAction(const NodeSharedPtr node, const tt_string& undo_str, bool AddToClipboard = false);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

    size_t GetMemorySize() override { return sizeof(*this); }

protected:
    void Init(const NodeSharedPtr node, tt_string_view undo_str, bool AddToClipboard = false);

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
    ModifyPropertyAction(NodeProperty* prop, tt_string_view value);
    ModifyPropertyAction(NodeProperty* prop, int value);
    void Change() override;
    void Revert() override;

    NodeProperty* GetProperty() override { return m_property; }

    // The +2 is to account for the trailing zero in each std::string value.
    size_t GetMemorySize() override { return sizeof(*this) + m_revert_value.size() + m_change_value.size() + 2; }

private:
    NodeProperty* m_property;
    tt_string m_revert_value;
    tt_string m_change_value;
};

// Used to modify multiple properties as a single undo/redo command.
//
// Specify undo string, and whether or not to fire events.
class ModifyProperties : public UndoAction
{
public:
    ModifyProperties(tt_string_view undo_string, bool fire_events = true);

    void AddProperty(NodeProperty* prop, tt_string_view value);
    void AddProperty(NodeProperty* prop, int value);

    void Change() override;
    void Revert() override;

    struct MULTI_PROP
    {
        NodeProperty* property;
        // All properties are stored as a string, no matter what their original data type
        tt_string revert_value;
        tt_string change_value;
    };
    auto& GetVector() { return m_properties; }

    size_t GetMemorySize() override;

private:
    std::vector<MULTI_PROP> m_properties;
    bool m_fire_events { true };
};

// Specify event and value.
class ModifyEventAction : public UndoAction
{
public:
    ModifyEventAction(NodeEvent* event, tt_string_view value);
    void Change() override;
    void Revert() override;

    size_t GetMemorySize() override { return sizeof(*this) + m_revert_value.size() + m_change_value.size(); }

private:
    NodeEvent* m_event;
    tt_string m_revert_value;
    tt_string m_change_value;
};

// Specify node and position.
class ChangePositionAction : public UndoAction
{
public:
    ChangePositionAction(Node* node, size_t position);
    ChangePositionAction(const NodeSharedPtr node, size_t position);

    void Change() override;
    void Revert() override;

    Node* GetParent() { return m_parent.get(); }
    Node* GetNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

protected:
    void Init(const NodeSharedPtr node, size_t position);

private:
    NodeSharedPtr m_parent;
    size_t m_change_pos;
    size_t m_revert_pos;
};

// Specify node and parent node.
class ChangeParentAction : public UndoAction
{
public:
    ChangeParentAction(Node* node, Node* parent);
    ChangeParentAction(const NodeSharedPtr node, const NodeSharedPtr parent);

    void Change() override;
    void Revert() override;

    Node* GetOldParent() { return m_revert_parent.get(); }
    Node* GetNewParent() { return m_change_parent.get(); }
    Node* GetNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

protected:
    void Init(const NodeSharedPtr node, const NodeSharedPtr parent);

private:
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

    size_t GetMemorySize() override { return sizeof(*this); }

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
    void Change() override;
    void Revert() override;

    Node* GetOldNode() { return m_old_node.get(); }
    Node* GetNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

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
    void Change() override;
    void Revert() override;

    size_t GetMemorySize() override { return sizeof(*this); }

private:
    NodeSharedPtr m_parent;
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
    GridBagAction(Node* cur_gbsizer, const tt_string& undo_str);
    void Change() override;
    void Revert() override;

    // Call this after making all changes to the gbsizer children
    void Update();
    Node* GetOldSizerNode() const { return m_old_gbsizer.get(); }
    Node* GetCurSizerNode() const { return m_cur_gbsizer.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

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

    size_t GetMemorySize() override { return sizeof(*this); }

protected:
    void SortFolder(Node* folder);

private:
    NodeSharedPtr m_old_project;
};

// This is used to check whether Update Images is at the top of the undo stack
extern const char* txt_update_images_undo_string;

// This is used when an Images List has the auto_update property changed
class AutoImagesAction : public UndoAction
{
public:
    AutoImagesAction(Node* node);
    void Change() override;
    void Revert() override;

    size_t GetMemorySize() override { return sizeof(*this); }

private:
    std::vector<UndoActionPtr> m_actions;
};
