/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// AI Context: This file implements Node, the core class representing a single UI component in
// wxUiEditor's project tree. Node uses shared_ptr semantics (NodeSharedPtr,
// enable_shared_from_this) for safe parent-child relationships stored in m_parent and m_children
// vectors. Each node references immutable metadata via m_declaration (NodeDeclaration*), stores
// user-configured properties in m_properties (vector<NodeProperty>) indexed by m_prop_indices map,
// and tracks events in m_map_events (unordered_map). Property access uses as_string/as_int/as_bool
// methods that delegate to NodeProperty, while HasValue/HasProp check existence. Node type queries
// (is_Form, is_Sizer, is_Gen) delegate to NodeDeclaration, and tree navigation uses
// get_Parent/get_Child/get_Form/get_Folder. Child management (AdoptChild, AddChild, RemoveChild)
// validates parent-child compatibility via NodeType rules and maintains tree consistency for
// undo/redo operations.

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace pugi
{
    class xml_document;
}

#include "font_prop.h"   // FontProperty class
#include "gen_enums.h"   // Enumerations for generators
#include "node_decl.h"   // NodeDeclaration class
#include "node_event.h"  // NodeEvent and NodeEventInfo classes
#include "node_prop.h"   // NodeProperty class

#include "hash_map.h"  // Find std::string_view key in std::unordered_map

#include "wxue_namespace/wxue_string.h"  // wxue::string class

class wxSizerFlags;
class wxAnimation;
struct ImageBundle;

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;
using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;

using namespace GenEnum;

// Structure to hold adjacent Node* parameters for parent and child.  This allows C++20 designated
// initializers to be used.
struct NodesParentChild
{
    Node* parent;
    Node* child;
};

class Node : public std::enable_shared_from_this<Node>
{
public:
    // node creation error codes
    enum class Validity : std::int8_t
    {
        valid_node = 0,
        unsupported_language = -1,
        unknown_gen_name = -2,
        parent_not_wxFrame = -3,
        invalid_tool_grandparent = -4,
        invalid_page_grandparent = -5,
        invalid_child_count = -6,
        gridbag_insert_error = -7,
        invalid_child = -8,  // Requested child is not allowed for the parent

        unknown_error = -99,
    };

    Node(NodeDeclaration* declaration);

    // Use get_name() if you want the enum value.
    auto get_DeclName() const noexcept { return m_declaration->get_DeclName(); }

    // Given a Node*, you can call this to get the std::shared_ptr<Node> for it.
    auto get_SharedPtr() -> NodeSharedPtr { return shared_from_this(); }

    auto get_ParentPtr() const noexcept -> NodeSharedPtr { return m_parent; }
    auto get_Parent() const noexcept -> Node* { return m_parent.get(); }

    auto get_ChildPtr(size_t index) -> NodeSharedPtr { return m_children.at(index); }
    auto get_Child(size_t index) const noexcept -> Node* { return m_children.at(index).get(); }
    auto get_ChildNodePtrs() -> auto& { return m_children; }
    auto get_ChildNodePtrs() const -> const auto& { return m_children; }

    void set_Parent(NodeSharedPtr parent) { m_parent = std::move(parent); }
    void set_Parent(Node* parent) { m_parent = parent->get_SharedPtr(); }

    auto get_PropPtr(PropName name) -> NodeProperty*;

    auto get_Event(std::string_view name) -> NodeEvent*;
    auto get_MapEvents() -> NodeMapEvents& { return m_map_events; }

    // Walk up the node tree looking for a container with a limited set of platforms. If
    // found, the container's node will be returned -- otherwise nullptr is returned.
    auto get_PlatformContainer() -> Node*;

    auto get_PropertyCount() const { return m_properties.size(); }
    auto get_InUseEventCount() const -> size_t;

    // Equivalent to AddChild(child); child->set_Parent(this);
    // Returns false if child is not allowed for this node.
    auto AdoptChild(const NodeSharedPtr& child) -> bool;

    auto AddChild(const NodeSharedPtr& node) -> bool;
    auto AddChild(Node* node) -> bool;
    auto AddChild(size_t idx, const NodeSharedPtr& node) -> bool;
    auto AddChild(size_t idx, Node* node) -> bool;

    // Returns the child's position or get_ChildCount() in case of not finding it
    auto get_ChildPosition(Node* node) -> size_t;
    auto get_ChildPosition(const NodeSharedPtr& node) -> size_t
    {
        return get_ChildPosition(node.get());
    }
    auto ChangeChildPosition(const NodeSharedPtr& node, size_t pos) -> bool;

    void RemoveChild(Node* node);
    void RemoveChild(const NodeSharedPtr& node) { RemoveChild(node.get()); }
    void RemoveChild(size_t index);
    void removeAllChildren() { m_children.clear(); }

    auto get_ChildCount() const { return m_children.size(); }

    auto is_ChildAllowed(Node* child) -> bool;
    auto is_ChildAllowed(NodeDeclaration* child) -> bool;
    auto is_ChildAllowed(const NodeSharedPtr& child) -> bool
    {
        return is_ChildAllowed(child->get_NodeDeclaration());
    }

    auto get_GenType() const { return m_declaration->get_GenType(); }

    // Returns the enum value for the name. Use get_DeclName() to get a char pointer.
    auto get_GenName() const -> GenName { return m_declaration->get_GenName(); }

    auto is_Type(GenType type) const noexcept -> bool
    {
        return (type == m_declaration->get_GenType());
    }
    auto is_Gen(GenName name) const noexcept -> bool
    {
        return (name == m_declaration->get_GenName());
    }
    auto is_Parent(GenName name) const noexcept -> bool
    {
        return (get_Parent() ? name == get_Parent()->m_declaration->get_GenName() : false);
    }

    // Returns true if this node is a folder, subfolder, Images List or Data List
    auto is_NonWidget() const noexcept -> bool
    {
        return (is_Gen(gen_folder) || is_Gen(gen_sub_folder) || is_Gen(gen_Images) ||
                is_Gen(gen_Data));
    }

    // Returns true if the node is either a folder or subfolder
    auto is_Folder() const noexcept -> bool
    {
        return (is_Gen(gen_folder) || is_Gen(gen_sub_folder) || is_Gen(gen_data_folder));
    }

    auto is_Widget() const noexcept -> bool { return is_Type(type_widget); }
    auto is_Wizard() const noexcept -> bool { return is_Type(type_wizard); }
    auto is_MenuBar() const noexcept -> bool
    {
        return (is_Type(type_menubar_form) || is_Type(type_menubar));
    }
    // Returns true if node is a menu or submenu
    auto is_Menu() const noexcept -> bool { return (is_Type(type_menu) || is_Type(type_submenu)); }
    auto is_ToolBar() const noexcept -> bool
    {
        return (is_Type(type_toolbar) || is_Type(type_toolbar_form) ||
                is_Type(type_aui_toolbar_form) || is_Type(type_aui_toolbar));
    }
    auto is_StatusBar() const noexcept -> bool { return is_Type(type_statusbar); }
    auto is_RibbonBar() const noexcept -> bool { return is_Type(type_ribbonbar); }

    auto is_Form() const noexcept -> bool;
    auto is_FormParent() const noexcept -> bool
    {
        return (is_Gen(gen_Project) || is_Gen(gen_folder) || is_Gen(gen_sub_folder));
    };

    auto is_StaticBoxSizer() const noexcept -> bool
    {
        return (is_Gen(gen_wxStaticBoxSizer) || is_Gen(gen_StaticCheckboxBoxSizer) ||
                is_Gen(gen_StaticRadioBtnBoxSizer));
    }
    auto is_Spacer() const noexcept -> bool { return is_Gen(gen_spacer); }

    auto is_Sizer() const noexcept -> bool
    {
        return (is_Type(type_sizer) || is_Type(type_gbsizer));
    }
    auto is_Container() const noexcept -> bool
    {
        if (is_Type(type_container) || is_Type(type_propsheetform) || is_Type(type_panel))
        {
            return true;
        }
        if (map_GenTypes.contains(get_GenType()))
        {
            return (map_GenTypes.at(get_GenType()).find("book") != std::string_view::npos);
        }
        return false;
    }

    // Returns true if access property == none or there is no access property
    auto is_Local() const noexcept -> bool;

    auto get_NodeType() { return m_declaration->get_NodeType(); }
    auto get_Generator() const { return m_declaration->get_Generator(); }

    // Returns the value of the property "var_name" or "class_name"
    auto get_NodeName() const -> const tt_string&;

    // May remove prefix based on the language -- e.g., @foo become foo unless the language
    // is GEN_LANG_RUBY
    auto get_NodeName(GenLang lang) const -> std::string_view;

    // Returns the value of the parent property "var_name" or "class_name"
    auto get_ParentName() const -> const tt_string&;

    // May remove prefix based on the language -- e.g., @foo become foo unless the language
    // is GEN_LANG_RUBY
    auto get_ParentName(GenLang lang, bool ignore_sizers = false) const -> std::string_view;

    // Returns this if the node is a form, else walks up node tree to find the parent form.
    auto get_Form() noexcept -> Node*;

    // Finds the parent form and returns the value of the it's property "class_name"
    auto get_FormName() -> const tt_string&;

    // Returns the folder node if there is one, nullptr otherwise.
    auto get_Folder() noexcept -> Node*;

    // This will walk up the parent tree until it finds a sub-folder, folder, or project
    // node. Use this to find a parent for a new form.
    auto get_ValidFormParent() noexcept -> Node*;

    auto get_NodeDeclaration() const -> NodeDeclaration* { return m_declaration; }

    // Retrieves prop_validator_data_type if it has one, or correct data type for use with
    // wxGenericValidator if it doesn't.
    auto get_ValidatorDataType() const -> std::string;

    // This will return wxGenericValidator, wxTextValidator, wxIntValidator or
    // wxFloatValidator
    auto get_ValidatorType() const -> std::string_view;

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    auto HasValue(PropName name) const -> bool;

    // Returns true if the property exists
    auto HasProp(PropName name) const -> bool { return (m_prop_indices.contains(name)); }

    // Avoid the temptation to use tt_string_view instead of const char* -- the MSVC compiler
    // will assume value is a bool if you call  is_PropValue(propm, "string")

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    auto is_PropValue(PropName name, const char* value) const noexcept -> bool;

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    auto is_PropValue(PropName name, bool value) const noexcept -> bool;

    auto is_PropValue(PropName name, int value) const noexcept -> bool;

    // Converts friendly name to wxWidgets constant, and then returns the integer value of
    // that constant.
    auto as_mockup(PropName name, std::string_view prefix) const -> int;

    // Use with caution! This allows you to modify the property string directly.
    //
    // Returns nullptr if the property doesn't exist.
    auto get_PropValuePtr(PropName name) -> tt_string*;

    auto get_PropDefaultValue(PropName name) -> std::string_view;

    // Sets value only if the property exists, returns false if it doesn't exist.
    template <typename T>
    auto set_value(PropName name, T value) -> bool
    {
        if (auto* prop = get_PropPtr(name); prop)
        {
            prop->set_value(value);
            return true;
        }

        return false;
    }

    // Returns string containing the property ID without any assignment if it is a custom id.
    auto get_PropId() const -> wxue::string;

    auto view(PropName name) const -> tt_string_view { return as_string(name); }

    auto as_bool(PropName name) const -> bool
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return (m_properties[result->second].as_string().atoi() != 0);
        }
        return false;
    }

    // If type is option, id, or bitlist, this will convert that constant name to it's value
    // (see NodeCreation.get_ConstantAsInt()). Otherwise, it calls atoi().
    auto as_int(PropName name) const -> int
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_int();
        }
        return 0;
    }

    auto as_constant(PropName name, std::string_view prefix) -> const tt_string&
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_constant(prefix);
        }
        return tt_empty_cstr;
    }

    // Looks up wx constant, returns it's numerical value.
    //
    // Returns wxID_ANY if constant is not found
    auto as_id(PropName name) const -> int
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_id();
        }

        return wxID_ANY;
    }

    auto as_double(PropName name) const -> double
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_float();
        }
        return 0;
    }

    auto as_string(PropName name) const -> const tt_string&
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_string();
        }
        return tt_empty_cstr;
    }

    auto as_view(PropName name) const -> std::string_view
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_view();
        }
        return {};
    }

    auto as_std(PropName name) const -> const std::string&
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_string();
        }
        return tt_empty_cstr;
    }

    // On Windows this will first convert to UTF-16 unless wxUSE_UNICODE_UTF8 is set.
    //
    // The string will be empty if the property doesn't exist.
    auto as_wxString(PropName name) const -> wxString
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_wxString();
        }
        return {};
    }

    auto as_wxBitmapBundle(PropName name) const -> wxBitmapBundle
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_bitmap_bundle();
        }
        return wxNullBitmap;
    }

    auto as_wxBitmap(PropName name) const -> wxBitmap
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_bitmap();
        }
        return wxNullBitmap;
    }

    auto as_wxColour(PropName name) const -> wxColour
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_color();
        }
        return {};
    }

    auto as_wxFont(PropName name) const -> wxFont
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_font();
        }
        return *wxNORMAL_FONT;
    }

    auto as_font_prop(PropName name) const -> FontProperty
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_font_prop();
        }
        return FontProperty(wxNORMAL_FONT);
    }

    auto as_wxPoint(PropName name) const -> wxPoint
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_point();
        }
        return wxDefaultPosition;
    }

    auto as_wxSize(PropName name) const -> wxSize
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_size();
        }
        return wxDefaultSize;
    }

    auto as_wxArrayString(PropName name) const -> wxArrayString
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_wxArrayString();
        }
        return {};
    }

    // Assumes all values are within quotes
    auto as_ArrayString(PropName name) const -> std::vector<wxue::string>;

    // If the following vector properties don't exist, they will return an empty vector

    auto as_statusbar_fields(PropName name) -> std::vector<NODEPROP_STATUSBAR_FIELD>;
    auto as_checklist_items(PropName name) -> std::vector<NODEPROP_CHECKLIST_ITEM>;
    auto as_radiobox_items(PropName name) -> std::vector<NODEPROP_RADIOBOX_ITEM>;
    auto as_bmp_combo_items(PropName name) -> std::vector<NODEPROP_BMP_COMBO_ITEM>;

    auto getSizerFlags() const -> wxSizerFlags;

    auto get_PropsVector() -> std::vector<NodeProperty>& { return m_properties; }

    auto AddNodeProperty(PropDeclaration* declaration) -> NodeProperty*;
    void AddNodeEvent(const NodeEventInfo* info);
    void CreateDoc(pugi::xml_document& doc);

    // This creates an orphaned node -- it is the caller's responsibility to hook it up with
    // a parent. Returns the node and an error code.
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    auto CreateChildNode(GenName name, bool verify_language_support = false, int pos = -1)
        -> std::pair<NodeSharedPtr, Node::Validity>;

    // Gets the current selected node and uses that to call CreateChildNode().
    auto CreateNode(GenName name) -> Node*;

    // This is the preferred way to create a new node when requested by the user (tool, menu,
    // or dialog). Besides creating the node, some nodes will get special processing to
    // automatically create additional child nodes.
    auto CreateToolNode(GenName name, int pos = -1) -> bool;

    // This will modify the property and fire a EVT_NodePropChange event if the property
    // actually changed
    void ModifyProperty(PropName name, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(tt_string_view name, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(tt_string_view name, int value);

    // This will modify the property and fire a EVT_NodePropChange event
    static void ModifyProperty(NodeProperty* prop, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    static void ModifyProperty(NodeProperty* prop, int value);

    // Both var_name and validator_variable properties are checked
    auto get_UniqueName(const std::string& proposed_name, PropName prop_name = prop_var_name)
        -> std::string;

    // Fix duplicate names in the current node and all of it's children
    void FixDuplicateNodeNames(Node* form = nullptr);

    auto FixDuplicateName() -> bool;

    // Collects all unique var_name, checkbox_var_name, radiobtn_var_name and
    // validator_variable properties in the current form
    //
    // If prop_name is != prop_var_name, only that property is collected.
    void CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node,
                            PropName prop_name = prop_var_name);

    auto FindInsertionPos(Node* child) const -> ptrdiff_t;
    auto FindInsertionPos(const NodeSharedPtr& child) const -> ptrdiff_t
    {
        return FindInsertionPos(child.get());
    }

    // Currently only called in debug builds, but available for release builds should we need it
    auto get_NodeSize() const -> size_t;

    // This writes XML files in the 1.1 layout using attributes for properties
    void AddNodeToDoc(pugi::xml_node& object, int& project_version);

    void CalcNodeHash(size_t& hash) const;

    auto get_AllowableChildren(GenType child_gen_type) const -> ptrdiff_t
    {
        return m_declaration->get_AllowableChildren(child_gen_type);
    }

    // Collect a vector of pointers to all children having the specified property with a
    // non-empty value.
    auto FindAllChildProperties(PropName name) -> std::vector<NodeProperty*>;

    void CopyEventsFrom(Node*);
    void CopyEventsFrom(const NodeSharedPtr& node) { CopyEventsFrom(node.get()); }

    void set_MockupObject(wxObject* object) { m_mockup_object = object; }
    auto get_MockupObject() const -> const wxObject* { return m_mockup_object; }

protected:
    void FindAllChildProperties(std::vector<NodeProperty*>& list, PropName name);

private:
    static auto GetBorderDirection(std::string_view border_settings) -> int;
    static auto ApplyAlignment(wxSizerFlags& flags, const tt_string& alignment) -> void;
    static auto ApplyAdditionalFlags(wxSizerFlags& flags, const tt_string& prop) -> void;

    // Helper methods for CreateChildNode to reduce complexity
    auto TryCreateInSizerChild(GenName name, bool verify_language_support, Node*& parent,
                               NodeSharedPtr& new_node) -> std::pair<NodeSharedPtr, Node::Validity>;
    static auto HandleGridBagInsertion(Node* parent, Node* new_node)
        -> std::pair<NodeSharedPtr, Node::Validity>;
    auto AdjustMemberNameForLanguage(Node* new_node) -> void;
    auto HandleRibbonButtonFallback([[maybe_unused]] GenName name, int pos)
        -> std::pair<NodeSharedPtr, Node::Validity>;
    auto TryCreateInParent(GenName name, [[maybe_unused]] int pos)
        -> std::pair<NodeSharedPtr, Node::Validity>;

    // Helper methods for FixDuplicateNodeNames to reduce complexity
    static auto InitializeNameSet(std::unordered_set<std::string>& name_set) -> void;
    static auto GenerateUniqueNameFromBase(const std::string& base_name,
                                           const std::unordered_set<std::string>& name_set)
        -> std::string;
    auto FixDuplicateVariableNames(const std::unordered_set<std::string>& name_set) -> void;
    auto FixChildrenNodeNames(Node* form) -> void;
    auto FixPropGridLabelIfNeeded() -> void;

    NodeSharedPtr m_parent;

    // Properties and events are added when the node is created, and then never changed for the
    // life of the node -- only the value of the property or event is changed.

    // The vector makes it possible to iterate through the properties in the order they were
    // created in the XML file. The map gives us a fast lookup into the vector.
    std::vector<NodeProperty> m_properties;
    std::map<PropName, size_t> m_prop_indices;

    // using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash,
    // std::equal_to<>>;
    std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>> m_map_events;

    std::vector<NodeSharedPtr> m_children;
    NodeDeclaration* m_declaration;

    wxObject* m_mockup_object { nullptr };
    std::unique_ptr<std::vector<tt_string>> m_internal_data;
};

using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;
