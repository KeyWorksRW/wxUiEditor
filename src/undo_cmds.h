/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

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

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

    // Set this to true if you created the node without firing a created event.
    void SetFireCreatedEvent(bool fire) { m_fire_created_event = fire; }

    size_t GetMemorySize() override { return sizeof(*this); }

    // The parent is read-only when the new node is inserted into a linked project.
    [[nodiscard]] bool IsReadOnly() const override;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_old_selected;
    int m_pos { -1 };
    bool m_fix_duplicate_names { true };
    bool m_fire_created_event { false };
};

// Specify node, undo string, and whether or not to add to the clipboard.
class RemoveNodeAction : public UndoAction
{
public:
    RemoveNodeAction(Node* node, std::string_view undo_str, bool AddToClipboard = false);

    // Called when pushed to the Undo stack and when Redo is called
    void Change() override;

    // Called when Undo is requested
    void Revert() override;

    size_t GetMemorySize() override { return sizeof(*this); }

    // The stored node or its parent is read-only when the node is removed from a linked project.
    [[nodiscard]] bool IsReadOnly() const override;

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
    void Change() override;
    void Revert() override;

    [[nodiscard]] NodeProperty* GetProperty() override { return m_property; }

    // The +2 is to account for the trailing zero in each std::string value.
    size_t GetMemorySize() override
    {
        return sizeof(*this) + m_revert_value.size() + m_change_value.size() + 2;
    }

    [[nodiscard]] bool IsReadOnly() const override;

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

    void addProperty(NodeProperty* prop, std::string_view value);
    void addProperty(NodeProperty* prop, int value);

    void Change() override;
    void Revert() override;

    struct MULTI_PROP
    {
        NodeProperty* property;
        // All properties are stored as a string, no matter what their original data type
        std::string revert_value;
        wxue::string change_value;
    };
    [[nodiscard]] auto& GetVector() { return m_properties; }

    size_t GetMemorySize() override;

    // True when any stored property belongs to a node inside a linked project.
    [[nodiscard]] bool IsReadOnly() const override;

private:
    std::vector<MULTI_PROP> m_properties;
    bool m_fire_events { true };

    // Set when one of the properties is an art directory, meaning every embedded image has to be
    // re-resolved after Change() or Revert(). See issue #1670.
    bool m_images_invalidated { false };
};

// Replaces the link_project value of a folder and reloads the linked file as a single undoable
// action. Used for linking, relinking and unlinking alike.
class LinkProjectAction : public UndoAction
{
public:
    // folder must be a gen_folder. new_value may be absolute or relative -- it is stored relative
    // to the project path, the same way a ModifyPropertyAction stores a file property. An empty
    // new_value unlinks the folder and removes the forms adopted from the linked file.
    LinkProjectAction(Node* folder, std::string_view new_value);

    void Change() override;
    void Revert() override;

    // The +2 is to account for the trailing zero in each wxue::string value.
    size_t GetMemorySize() override
    {
        return sizeof(*this) + m_revert_value.size() + m_change_value.size() + 2;
    }

    // A link folder itself is always editable -- only the content it adopts is read-only.
    [[nodiscard]] bool IsReadOnly() const override { return false; }

private:
    // Sets link_project to value and reloads. When the load is refused (the user declined to
    // replace the existing children) or fails validation, fallback is restored so the property
    // grid matches the tree. replace_silently is used by undo/redo, where the user already asked
    // for the state change and must not be prompted again.
    void ApplyLink(const wxue::string& value, const wxue::string& fallback, bool replace_silently);

    NodeSharedPtr m_folder;
    wxue::string m_revert_value;
    wxue::string m_change_value;
};

// Specify event and value.
class ModifyEventAction : public UndoAction
{
public:
    ModifyEventAction(NodeEvent* event, std::string_view value);
    void Change() override;
    void Revert() override;

    size_t GetMemorySize() override
    {
        return sizeof(*this) + m_revert_value.size() + m_change_value.size();
    }

    [[nodiscard]] bool IsReadOnly() const override;

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

    void Change() override;
    void Revert() override;

    [[nodiscard]] Node* get_Parent() { return m_parent.get(); }
    [[nodiscard]] Node* getNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

    [[nodiscard]] bool IsReadOnly() const override;

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

    void Change() override;
    void Revert() override;

    [[nodiscard]] Node* GetOldParent() { return m_revert_parent.get(); }
    [[nodiscard]] Node* GetNewParent() { return m_change_parent.get(); }
    [[nodiscard]] Node* getNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

    // The moved node and both its old and new parent must be editable.
    [[nodiscard]] bool IsReadOnly() const override;

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
    void Change() override;
    void Revert() override;

    [[nodiscard]] NodeSharedPtr GetOldNode() override { return m_old_node; }
    [[nodiscard]] Node* getNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

    [[nodiscard]] bool IsReadOnly() const override;

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

    [[nodiscard]] NodeSharedPtr GetOldNode() override { return m_old_node; }
    [[nodiscard]] Node* getNode() { return m_node.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

    [[nodiscard]] bool IsReadOnly() const override;

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

    // The parent is read-only when the node is appended to a linked project.
    [[nodiscard]] bool IsReadOnly() const override;

private:
    NodeSharedPtr m_parent;
    NodeSharedPtr m_old_selected;

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
    void Change() override;
    void Revert() override;

    // Call this after making all changes to the gbsizer children
    void Update();
    [[nodiscard]] Node* GetOldSizerNode() const { return m_old_gbsizer.get(); }
    [[nodiscard]] Node* GetCurSizerNode() const { return m_cur_gbsizer.get(); }

    size_t GetMemorySize() override { return sizeof(*this); }

    [[nodiscard]] bool IsReadOnly() const override;

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
    [[nodiscard]] NodeSharedPtr GetOldNode() override { return m_old_project; }

protected:
    static void SortFolder(Node* folder);

private:
    NodeSharedPtr m_old_project;
};
