/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <unordered_map>
#include <unordered_set>
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
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count

class wxSizerFlags;
class wxAnimation;
struct ImageBundle;

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;
using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;

using namespace GenEnum;

class Node : public std::enable_shared_from_this<Node>
{
public:
    Node(NodeDeclaration* declaration);

    // Use get_name() if you want the enum value.
    auto declName() const noexcept { return m_declaration->declName(); }

    // Given a Node*, you can call this to get the std::shared_ptr<Node> for it.
    NodeSharedPtr getSharedPtr() { return shared_from_this(); }

    NodeSharedPtr getParentPtr() const noexcept { return m_parent; }
    Node* getParent() const noexcept { return m_parent.get(); }

    NodeSharedPtr getChildPtr(size_t index) { return m_children.at(index); }
    Node* getChild(size_t index) const noexcept { return m_children.at(index).get(); }
    auto& getChildNodePtrs() { return m_children; }

    void setParent(NodeSharedPtr parent) { m_parent = parent; }
    void setParent(Node* parent) { m_parent = parent->getSharedPtr(); }

    NodeProperty* getPropPtr(PropName name);

    NodeEvent* getEvent(tt_string_view name);
    NodeMapEvents& getMapEvents() { return m_map_events; }

    // Walk up the node tree looking for a container with a limited set of platforms. If
    // found, the container's node will be returned -- otherwise nullptr is returned.
    Node* getPlatformContainer();

    auto getPropertyCount() const { return m_properties.size(); }
    size_t getInUseEventCount() const;

    // Equivalent to addChild(child); child->setParent(this);
    // Returns false if child is not allowed for this node.
    bool adoptChild(NodeSharedPtr child);

    bool addChild(NodeSharedPtr node);
    bool addChild(Node* node);
    bool addChild(size_t index, NodeSharedPtr node);
    bool addChild(size_t index, Node* node);

    // Returns the child's position or getChildCount() in case of not finding it
    size_t getChildPosition(Node* node);
    size_t getChildPosition(const NodeSharedPtr& node) { return getChildPosition(node.get()); }
    bool changeChildPosition(NodeSharedPtr node, size_t pos);

    void removeChild(Node* node);
    void removeChild(const NodeSharedPtr& node) { removeChild(node.get()); }
    void removeChild(size_t index);
    void removeAllChildren() { m_children.clear(); }

    auto getChildCount() const { return m_children.size(); }

    bool isChildAllowed(Node* child);
    bool isChildAllowed(NodeDeclaration* child);
    bool isChildAllowed(const NodeSharedPtr& child) { return isChildAllowed(child->getNodeDeclaration()); }

    auto getGenType() const { return m_declaration->getGenType(); }

    // Returns the enum value for the name. Use declName() to get a char pointer.
    GenName getGenName() const { return m_declaration->getGenName(); }

    bool isType(GenType type) const noexcept { return (type == m_declaration->getGenType()); }
    bool isGen(GenName name) const noexcept { return (name == m_declaration->getGenName()); }
    bool isParent(GenName name) const noexcept
    {
        return (getParent() ? name == getParent()->m_declaration->getGenName() : false);
    }

    // Returns true if this node is a folder, subfolder, or Images List
    bool isNonWidget() const noexcept { return (isGen(gen_folder) || isGen(gen_sub_folder) || isGen(gen_Images)); }

    // Returns true if the node is either a folder or subfolder
    bool isFolder() const noexcept { return (isGen(gen_folder) || isGen(gen_sub_folder)); }

    bool isWidget() const noexcept { return isType(type_widget); }
    bool isWizard() const noexcept { return isType(type_wizard); }
    bool isMenuBar() const noexcept { return (isType(type_menubar_form) || isType(type_menubar)); }
    bool isToolBar() const noexcept
    {
        return (isType(type_toolbar) || isType(type_toolbar_form) || isType(type_aui_toolbar_form) ||
                isType(type_aui_toolbar));
    }
    bool isStatusBar() const noexcept { return isType(type_statusbar); }
    bool isRibbonBar() const noexcept { return isType(type_ribbonbar); }

    bool isForm() const noexcept;
    bool isFormParent() const noexcept { return (isGen(gen_Project) || isGen(gen_folder) || isGen(gen_sub_folder)); };

    bool isStaticBoxSizer() const noexcept
    {
        return (isGen(gen_wxStaticBoxSizer) || isGen(gen_StaticCheckboxBoxSizer) || isGen(gen_StaticRadioBtnBoxSizer));
    }
    bool isSpacer() const noexcept { return isGen(gen_spacer); }

    bool isSizer() const noexcept { return (isType(type_sizer) || isType(type_gbsizer)); }
    bool isContainer() const noexcept
    {
        return (isType(type_container) || isType(type_propsheetform) || tt::contains(map_GenTypes[getGenType()], "book"));
    }

    // Returns true if access property == none or there is no access property
    bool isLocal() const noexcept;

    auto getNodeType() { return m_declaration->getNodeType(); }
    auto getGenerator() const { return m_declaration->getGenerator(); }

    // Returns the value of the property "var_name" or "class_name"
    const tt_string& getNodeName() const;

    // Returns the value of the parent property "var_name" or "class_name"
    const tt_string& getParentName() const;

    // Returns this if the node is a form, else walks up node tree to find the parent form.
    Node* getForm() noexcept;

    // Finds the parent form and returns the value of the it's property "class_name"
    const tt_string& getFormName();

    // Returns the folder node if there is one, nullptr otherwise.
    Node* getFolder() noexcept;

    // This will walk up the parent tree until it finds a sub-folder, folder, or project
    // node. Use this to find a parent for a new form.
    Node* getValidFormParent() noexcept;

    NodeDeclaration* getNodeDeclaration() { return m_declaration; }

    // Retrieves prop_validator_data_type if it has one, or correct data type for use with
    // wxGenericValidator if it doesn't.
    tt_string getValidatorDataType() const;

    // This will return wxGenericValidator, wxTextValidator, wxIntValidator or
    // wxFloatValidator
    tt_string_view getValidatorType() const;

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    bool hasValue(PropName name) const;

    // Returns true if the property exists
    bool hasProp(PropName name) const { return (m_prop_indices.find(name) != m_prop_indices.end()); }

    // Avoid the temptation to use tt_string_view instead of const char* -- the MSVC compiler
    // will assume value is a bool if you call  isPropValue(propm, "string")

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool isPropValue(PropName name, const char* value) const noexcept;

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool isPropValue(PropName name, bool value) const noexcept;

    bool isPropValue(PropName name, int value) const noexcept;

    // Converts friendly name to wxWidgets constant, and then returns the integer value of
    // that constant.
    int as_mockup(PropName name, std::string_view prefix) const;

    // Use with caution! This allows you to modify the property string directly.
    //
    // Returns nullptr if the property doesn't exist.
    tt_string* getPropValuePtr(PropName name);

    const tt_string& getPropDefaultValue(PropName name);

    // Sets value only if the property exists, returns false if it doesn't exist.
    template <typename T>
    bool set_value(PropName name, T value)
    {
        if (auto prop = getPropPtr(name); prop)
        {
            prop->set_value(value);
            return true;
        }
        else
        {
            return false;
        }
    }

    // Returns string containing the property ID without any assignment if it is a custom id.
    tt_string getPropId() const;

    const tt_string_view view(PropName name) const { return as_string(name); }

    bool as_bool(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return (m_properties[result->second].as_string().atoi() != 0);
        else
            return false;
    }

    // If type is option, id, or bitlist, this will convert that constant name to it's value
    // (see NodeCreation.getConstantAsInt()). Otherwise, it calls atoi().
    int as_int(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_int();
        else
            return 0;
    }

    const tt_string& as_constant(PropName name, std::string_view prefix)
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_constant(prefix);
        else
            return tt_empty_cstr;
    }

    // Looks up wx constant, returns it's numerical value.
    //
    // Returns wxID_ANY if constant is not found
    int as_id(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_id();
        else
            return wxID_ANY;
    }

    double as_double(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_float();
        else
            return 0;
    }

    const tt_string& as_string(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_string();
        else
            return tt_empty_cstr;
    }

    // On Windows this will first convert to UTF-16 unless wxUSE_UNICODE_UTF8 is set.
    //
    // The string will be empty if the property doesn't exist.
    wxString as_wxString(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_wxString();
        else
            return {};
    }

    wxBitmapBundle as_wxBitmapBundle(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_bitmap_bundle();
        else
            return wxNullBitmap;
    }

    const ImageBundle* as_image_bundle(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_image_bundle();
        else
            return nullptr;
    }

    wxBitmap as_wxBitmap(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_bitmap();
        else
            return wxNullBitmap;
    }

    wxColour as_wxColour(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_color();
        else
            return {};
    }

    wxFont as_wxFont(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_font();
        else
            return *wxNORMAL_FONT;
    }

    FontProperty as_font_prop(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_font_prop();
        else
            return FontProperty(wxNORMAL_FONT);
    }

    wxPoint as_wxPoint(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_point();
        else
            return wxDefaultPosition;
    }

    wxSize as_wxSize(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_size();
        else
            return wxDefaultSize;
    }

    wxArrayString as_wxArrayString(PropName name) const
    {
        if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
            return m_properties[result->second].as_wxArrayString();
        else
            return {};
    }

    // Assumes all values are within quotes
    std::vector<tt_string> as_ArrayString(PropName name) const;

    // If the following vector properties don't exist, they will return an empty vector

    std::vector<NODEPROP_STATUSBAR_FIELD> as_statusbar_fields(PropName name);
    std::vector<NODEPROP_CHECKLIST_ITEM> as_checklist_items(PropName name);
    std::vector<NODEPROP_RADIOBOX_ITEM> as_radiobox_items(PropName name);
    std::vector<NODEPROP_BMP_COMBO_ITEM> as_bmp_combo_items(PropName name);

    wxSizerFlags getSizerFlags() const;

    std::vector<NodeProperty>& getPropsVector() { return m_properties; }

    NodeProperty* addNodeProperty(PropDeclaration* info);
    void addNodeEvent(const NodeEventInfo* info);
    void createDoc(pugi::xml_document& doc);

    // This creates an orphaned node -- it is the caller's responsibility to hook it up with
    // a parent.
    Node* createChildNode(GenName name);

    // Gets the current selected node and uses that to call createChildNode().
    Node* createNode(GenName name);

    // This is the preferred way to create a new node when requested by the user (tool, menu,
    // or dialog). Besides creating the node, some nodes will get special processing to
    // automatically create additional child nodes.
    bool createToolNode(GenName name);

    // This will modify the property and fire a EVT_NodePropChange event if the property
    // actually changed
    void modifyProperty(PropName name, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void modifyProperty(tt_string_view name, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void modifyProperty(tt_string_view name, int value);

    // This will modify the property and fire a EVT_NodePropChange event
    void modifyProperty(NodeProperty* prop, tt_string_view value);

    // This will modify the property and fire a EVT_NodePropChange event
    void modifyProperty(NodeProperty* prop, int value);

    // Both var_name and validator_variable properties are checked
    tt_string getUniqueName(const tt_string& proposed_name, PropName prop_name = prop_var_name);

    // Fix duplicate names in the current node and all of it's children
    void fixDuplicateNodeNames(Node* form = nullptr);

    bool fixDuplicateName();

    // Collects all unique var_name, checkbox_var_name, radiobtn_var_name and
    // validator_variable properties in the current form
    //
    // If prop_name is != prop_var_name, only that property is collected.
    void collectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node, PropName prop_name = prop_var_name);

    ptrdiff_t findInsertionPos(Node* child) const;
    ptrdiff_t findInsertionPos(const NodeSharedPtr& child) const { return findInsertionPos(child.get()); }

    // Currently only called in debug builds, but available for release builds should we need it
    size_t getNodeSize() const;

    // This writes XML files in the 1.1 layout using attributes for properties
    void addNodeToDoc(pugi::xml_node& object, int& project_version);

    void calcNodeHash(size_t& hash) const;

    ptrdiff_t getAllowableChildren(GenType child_gen_type) const
    {
        return m_declaration->getAllowableChildren(child_gen_type);
    }

    // Collect a vector of pointers to all children having the specified property with a
    // non-empty value.
    std::vector<NodeProperty*> findAllChildProperties(PropName name);

    void copyEventsFrom(Node*);
    void copyEventsFrom(const NodeSharedPtr& node) { return copyEventsFrom(node.get()); }

    void setMockupObject(wxObject* object) { m_mockup_object = object; }
    const wxObject* getMockupObject() const { return m_mockup_object; }

    // This will create a std::unique_ptr<std::vector<tt_string>> if one doesn't already exist.
    std::vector<tt_string>* getInternalData();

protected:
    void findAllChildProperties(std::vector<NodeProperty*>& list, PropName name);

private:
    NodeSharedPtr m_parent;

    // Properties and events are added when the node is created, and then never changed for the life of the node --
    // only the value of the property or event is changed.

    // The vector makes it possible to iterate through the properties in the order they were created in the XML file. The
    // map gives us a fast lookup into the vector.
    std::vector<NodeProperty> m_properties;
    std::map<PropName, size_t> m_prop_indices;

    // using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;
    std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>> m_map_events;

    std::vector<NodeSharedPtr> m_children;
    NodeDeclaration* m_declaration;

    wxObject* m_mockup_object { nullptr };
    std::unique_ptr<std::vector<tt_string>> m_internal_data;
};

using NodeMapEvents = std::unordered_map<std::string, NodeEvent, str_view_hash, std::equal_to<>>;
