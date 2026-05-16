/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>
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
    // --- Lifecycle & Construction ---

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

    // --- Parent-Child Hierarchy ---

    // Given a Node*, you can call this to get the std::shared_ptr<Node> for it.
    NodeSharedPtr get_SharedPtr() { return shared_from_this(); }

    NodeSharedPtr get_ParentPtr() const noexcept { return m_parent; }
    Node* get_Parent() const noexcept { return m_parent.get(); }

    NodeSharedPtr get_ChildPtr(size_t index) { return m_children.at(index); }
    Node* get_Child(size_t index) const noexcept { return m_children.at(index).get(); }
    std::vector<NodeSharedPtr>& get_ChildNodePtrs() { return m_children; }
    const std::vector<NodeSharedPtr>& get_ChildNodePtrs() const { return m_children; }

    void set_Parent(NodeSharedPtr parent) { m_parent = std::move(parent); }
    void set_Parent(Node* parent) { m_parent = parent->get_SharedPtr(); }

    // --- Property Access ---

    NodeProperty* get_PropPtr(PropName name);

    // --- Event Access ---

    NodeEvent* get_Event(std::string_view name);
    NodeMapEvents& get_MapEvents() { return m_map_events; }

    auto get_PropertyCount() const { return m_properties.size(); }
    size_t get_InUseEventCount() const;

    // Equivalent to AddChild(child); child->set_Parent(this);
    // Returns false if child is not allowed for this node.
    bool AdoptChild(const NodeSharedPtr& child);

    bool AddChild(const NodeSharedPtr& node);
    bool AddChild(Node* node);
    bool AddChild(size_t idx, const NodeSharedPtr& node);
    bool AddChild(size_t idx, Node* node);

    // Returns the child's position or get_ChildCount() in case of not finding it
    size_t get_ChildPosition(Node* node);
    size_t get_ChildPosition(const NodeSharedPtr& node) { return get_ChildPosition(node.get()); }
    bool ChangeChildPosition(const NodeSharedPtr& node, size_t pos);

    void RemoveChild(Node* node);
    void RemoveChild(const NodeSharedPtr& node) { RemoveChild(node.get()); }
    void RemoveChild(size_t index);
    void removeAllChildren() { m_children.clear(); }

    auto get_ChildCount() const { return m_children.size(); }

    bool is_ChildAllowed(Node* child);
    bool is_ChildAllowed(NodeDeclaration* child);
    bool is_ChildAllowed(const NodeSharedPtr& child)
    {
        return is_ChildAllowed(child->get_NodeDeclaration());
    }

    // --- Type Introspection ---

    auto get_GenType() const { return m_declaration->get_GenType(); }

    // Returns the enum value for the name. Use get_DeclName() to get a char pointer.
    GenName get_GenName() const { return m_declaration->get_GenName(); }

    bool is_Type(GenType type) const noexcept { return (type == m_declaration->get_GenType()); }
    bool is_Gen(GenName name) const noexcept { return (name == m_declaration->get_GenName()); }
    bool is_Parent(GenName name) const noexcept
    {
        return (get_Parent() ? name == get_Parent()->m_declaration->get_GenName() : false);
    }

    // Returns true if this node is a folder, subfolder, Images List or Data List
    bool is_NonWidget() const noexcept
    {
        return (is_Gen(gen_folder) || is_Gen(gen_sub_folder) || is_Gen(gen_Images) ||
                is_Gen(gen_Data));
    }

    // Returns true if the node is either a folder or subfolder
    bool is_Folder() const noexcept
    {
        return (is_Gen(gen_folder) || is_Gen(gen_sub_folder) || is_Gen(gen_data_folder));
    }

    bool is_Widget() const noexcept { return is_Type(type_widget); }
    bool is_Wizard() const noexcept { return is_Type(type_wizard); }
    bool is_MenuBar() const noexcept
    {
        return (is_Type(type_menubar_form) || is_Type(type_menubar));
    }
    // Returns true if node is a menu or submenu
    bool is_Menu() const noexcept { return (is_Type(type_menu) || is_Type(type_submenu)); }
    bool is_ToolBar() const noexcept
    {
        return (is_Type(type_toolbar) || is_Type(type_toolbar_form) ||
                is_Type(type_aui_toolbar_form) || is_Type(type_aui_toolbar));
    }
    bool is_StatusBar() const noexcept { return is_Type(type_statusbar); }
    bool is_RibbonBar() const noexcept { return is_Type(type_ribbonbar); }

    bool is_Form() const noexcept;
    bool is_FormParent() const noexcept
    {
        return (is_Gen(gen_Project) || is_Gen(gen_folder) || is_Gen(gen_sub_folder));
    };

    bool is_StaticBoxSizer() const noexcept
    {
        return (is_Gen(gen_wxStaticBoxSizer) || is_Gen(gen_StaticCheckboxBoxSizer) ||
                is_Gen(gen_StaticRadioBtnBoxSizer));
    }
    bool is_Spacer() const noexcept { return is_Gen(gen_spacer); }

    bool is_Sizer() const noexcept { return (is_Type(type_sizer) || is_Type(type_gbsizer)); }
    bool is_Container() const noexcept
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
    bool is_Local() const noexcept;

    auto get_NodeType() { return m_declaration->get_NodeType(); }
    auto get_Generator() const { return m_declaration->get_Generator(); }

    // --- Node Naming & Navigation ---

    // Returns the value of the property "var_name" or "class_name"
    std::string_view get_NodeName() const;

    // May remove prefix based on the language -- e.g., @foo become foo unless the language
    // is GEN_LANG_RUBY
    std::string_view get_NodeName(GenLang lang) const;

    // Returns the value of the parent property "var_name" or "class_name"
    std::string_view get_ParentName() const;

    // May remove prefix based on the language -- e.g., @foo become foo unless the language
    // is GEN_LANG_RUBY
    std::string_view get_ParentName(GenLang lang, bool ignore_sizers = false) const;

    // Returns this if the node is a form, else walks up node tree to find the parent form.
    Node* get_Form() noexcept;

    // Finds the parent form and returns the value of the it's property "class_name"
    std::string_view get_FormName();

    // Returns the folder node if there is one, nullptr otherwise.
    Node* get_Folder() noexcept;

    // This will walk up the parent tree until it finds a sub-folder, folder, or project
    // node. Use this to find a parent for a new form.
    Node* get_ValidFormParent() noexcept;

    NodeDeclaration* get_NodeDeclaration() const { return m_declaration; }

    // Walk up the node tree looking for a container with a limited set of platforms. If
    // found, the container's node will be returned -- otherwise nullptr is returned.
    Node* get_PlatformContainer();

    // --- Validator Support ---

    // Retrieves prop_validator_data_type if it has one, or correct data type for use with
    // wxGenericValidator if it doesn't.
    std::string get_ValidatorDataType() const;

    // This will return wxGenericValidator, wxTextValidator, wxIntValidator or
    // wxFloatValidator
    std::string_view get_ValidatorType() const;

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-specified bitmap)
    bool HasValue(PropName name) const;

    // Returns true if the property exists
    bool HasProp(PropName name) const { return (m_prop_indices.contains(name)); }

    // --- Property Checks & Modification ---

    // Avoid the temptation to use wxue::string_view instead of const char* -- the MSVC compiler
    // will assume value is a bool if you call  is_PropValue(propm, "string")

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool is_PropValue(PropName name, const char* value) const noexcept;

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool is_PropValue(PropName name, bool value) const noexcept;

    bool is_PropValue(PropName name, int value) const noexcept;

    // Converts friendly name to wxWidgets constant, and then returns the integer value of
    // that constant.
    int as_mockup(PropName name, std::string_view prefix) const;

    // Use with caution! This allows you to modify the property string directly.
    //
    // Returns nullptr if the property doesn't exist.
    wxue::string* get_PropValuePtr(PropName name);

    std::string_view get_PropDefaultValue(PropName name);

    // Sets value only if the property exists, returns false if it doesn't exist.
    template <typename T>
    bool set_value(PropName name, T value)
    {
        if (NodeProperty* prop = get_PropPtr(name); prop)
        {
            prop->set_value(value);
            return true;
        }

        return false;
    }

    // Returns string containing the property ID without any assignment if it is a custom id.
    wxue::string get_PropId() const;

    // --- Property Value Queries ---

    wxue::string_view view(PropName name) const { return as_view(name); }

    bool as_bool(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return (m_properties[result->second].as_string().atoi() != 0);
        }
        return false;
    }

    // If type is option, id, or bitlist, this will convert that constant name to it's value
    // (see NodeCreation.get_ConstantAsInt()). Otherwise, it calls atoi().
    int as_int(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_int();
        }
        return 0;
    }

    const wxue::string& as_constant(PropName name, std::string_view prefix)
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_constant(prefix);
        }
        return wxue::wxue_empty_string;
    }

    // Looks up wx constant, returns it's numerical value.
    //
    // Returns wxID_ANY if constant is not found
    int as_id(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_id();
        }

        return wxID_ANY;
    }

    double as_double(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_float();
        }
        return 0;
    }

    const wxue::string& as_string(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_string();
        }
        return wxue::wxue_empty_string;
    }

    std::string_view as_view(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_view();
        }
        return {};
    }

    const std::string& as_std(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_string();
        }
        return wxue::wxue_empty_string;
    }

    // On Windows this will first convert to UTF-16 unless wxUSE_UNICODE_UTF8 is set.
    //
    // The string will be empty if the property doesn't exist.
    wxString as_wxString(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_wxString();
        }
        return {};
    }

    wxBitmapBundle as_wxBitmapBundle(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_bitmap_bundle();
        }
        return wxNullBitmap;
    }

    wxBitmap as_wxBitmap(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_bitmap();
        }
        return wxNullBitmap;
    }

    wxColour as_wxColour(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_color();
        }
        return {};
    }

    wxFont as_wxFont(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_font();
        }
        return *wxNORMAL_FONT;
    }

    FontProperty as_font_prop(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_font_prop();
        }
        return FontProperty(wxNORMAL_FONT);
    }

    wxPoint as_wxPoint(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_point();
        }
        return wxDefaultPosition;
    }

    wxSize as_wxSize(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_size();
        }
        return wxDefaultSize;
    }

    wxArrayString as_wxArrayString(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        {
            return m_properties[result->second].as_wxArrayString();
        }
        return {};
    }

    // Assumes all values are within quotes
    std::vector<wxue::string> as_ArrayString(PropName name) const;

    // If the following vector properties don't exist, they will return an empty vector

    std::vector<NODEPROP_STATUSBAR_FIELD> as_statusbar_fields(PropName name);
    std::vector<NODEPROP_CHECKLIST_ITEM> as_checklist_items(PropName name);
    std::vector<NODEPROP_RADIOBOX_ITEM> as_radiobox_items(PropName name);
    std::vector<NODEPROP_BMP_COMBO_ITEM> as_bmp_combo_items(PropName name);

    // --- Sizer Support ---

    wxSizerFlags getSizerFlags() const;

    // --- Node Construction ---

    std::vector<NodeProperty>& get_PropsVector() { return m_properties; }

    NodeProperty* AddNodeProperty(PropDeclaration* declaration);
    void AddNodeEvent(const NodeEventInfo* info);
    void CreateDoc(pugi::xml_document& xml_doc);

    // This creates an orphaned node -- it is the caller's responsibility to hook it up with
    // a parent. Returns the node and an error code.
    //
    // If verify_language_support is true, then the node will only be created if the
    // preferred language supports it (unless the user agrees to create it anyway)
    std::pair<NodeSharedPtr, Node::Validity>
        CreateChildNode(GenName name, bool verify_language_support = false, int pos = -1);

    // Gets the current selected node and uses that to call CreateChildNode().
    Node* CreateNode(GenName name);

    // This is the preferred way to create a new node when requested by the user (tool, menu,
    // or dialog). Besides creating the node, some nodes will get special processing to
    // automatically create additional child nodes.
    std::expected<bool, std::string> CreateToolNode(GenName name, int pos = -1);

    // This will modify the property and fire a EVT_NodePropChange event if the property
    // actually changed
    void ModifyProperty(PropName name, wxue::string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(wxue::string_view name, wxue::string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(wxue::string_view name, int value);

    // This will modify the property and fire a EVT_NodePropChange event
    static void ModifyProperty(NodeProperty* prop, wxue::string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    static void ModifyProperty(NodeProperty* prop, int value);

    // --- Name Uniqueness ---

    // Both var_name and validator_variable properties are checked
    std::string get_UniqueName(const std::string& proposed_name,
                               PropName prop_name = prop_var_name);

    // Fix duplicate names in the current node and all of it's children
    void FixDuplicateNodeNames(Node* form = nullptr);

    bool FixDuplicateName();

    // Collects all unique var_name, checkbox_var_name, radiobtn_var_name and
    // validator_variable properties in the current form
    //
    // If prop_name is != prop_var_name, only that property is collected.
    void CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node,
                            PropName prop_name = prop_var_name);

    // --- Insertion & Position ---

    ptrdiff_t FindInsertionPos(Node* child) const;
    ptrdiff_t FindInsertionPos(const NodeSharedPtr& child) const
    {
        return FindInsertionPos(child.get());
    }

    // --- Serialization & Hashing ---

    // Currently only called in debug builds, but available for release builds should we need it
    size_t get_NodeSize() const;

    // This writes XML files in the 1.1 layout using attributes for properties
    void AddNodeToDoc(pugi::xml_node& object, int& project_version);

    void CalcNodeHash(size_t& hash) const;

    ptrdiff_t get_AllowableChildren(GenType child_gen_type) const
    {
        return m_declaration->get_AllowableChildren(child_gen_type);
    }

    // Collect a vector of pointers to all children having the specified property with a
    // non-empty value.
    std::vector<NodeProperty*> FindAllChildProperties(PropName name);

    // --- Event Access ---

    void CopyEventsFrom(Node*);
    void CopyEventsFrom(const NodeSharedPtr& node) { CopyEventsFrom(node.get()); }

    // --- Mockup & Editing Operations ---

    void set_MockupObject(wxObject* object) { m_mockup_object = object; }
    const wxObject* get_MockupObject() const { return m_mockup_object; }

    void RemoveNode(bool isCutMode);
    void CopyNode();
    void DuplicateNode();
    void ToggleBorderFlag(int border);
    void ChangeAlignment(int align, bool vertical);

    bool MoveNode(MoveDirection where, bool check_only = false);

    // parent must be a sizer and the current node must be a sizer
    bool MoveSizerChildrenToParent();

protected:
    void FindAllChildProperties(std::vector<NodeProperty*>& list, PropName name);

private:
    static int GetBorderDirection(std::string_view border_settings);
    static void ApplyAlignment(wxSizerFlags& flags, const wxue::string& alignment);
    static void ApplyAdditionalFlags(wxSizerFlags& flags, const wxue::string& prop);

    // Helper methods for CreateChildNode to reduce complexity
    std::pair<NodeSharedPtr, Node::Validity> TryCreateInSizerChild(GenName name,
                                                                   bool verify_language_support,
                                                                   Node*& parent,
                                                                   NodeSharedPtr& new_node);
    static std::pair<NodeSharedPtr, Node::Validity> HandleGridBagInsertion(Node* parent,
                                                                           Node* new_node);
    void AdjustMemberNameForLanguage(Node* new_node);
    std::pair<NodeSharedPtr, Node::Validity>
        HandleRibbonButtonFallback([[maybe_unused]] GenName name, int pos);
    std::pair<NodeSharedPtr, Node::Validity> TryCreateInParent(GenName name,
                                                               [[maybe_unused]] int pos);

    // Helper methods for CreateToolNode to reduce complexity
    void AdjustNameForFrameForm(GenName& name);
    bool CreateFolderNode(GenName& name);
    bool CreateImagesListNode();
    bool CreateDataListNode();
    void PostProcessNewNode(NodeSharedPtr& new_node, GenName name);

    // Helper methods for FixDuplicateNodeNames to reduce complexity
    static void InitializeNameSet(std::unordered_set<std::string>& name_set);
    static std::string GenerateUniqueNameFromBase(const std::string& base_name,
                                                  const std::unordered_set<std::string>& name_set);
    void FixDuplicateVariableNames(const std::unordered_set<std::string>& name_set);
    void FixChildrenNodeNames(Node* form);
    void FixPropGridLabelIfNeeded();

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
    std::unique_ptr<std::vector<wxue::string>> m_internal_data;
};

using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;
