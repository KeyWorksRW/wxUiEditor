/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
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

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;

using namespace GenEnum;

class Node : public std::enable_shared_from_this<Node>
{
public:
    Node(NodeDeclaration* declaration);

    // Use get_name() if you want the enum value.
    auto DeclName() const noexcept { return m_declaration->DeclName(); }

    NodeSharedPtr GetParentPtr() const noexcept { return m_parent; }
    Node* GetParent() const noexcept { return m_parent.get(); }

    void SetParent(NodeSharedPtr parent) { m_parent = parent; }
    void SetParent(Node* parent) { m_parent = parent->GetSharedPtr(); }

    NodeProperty* get_prop_ptr(PropName name);

    NodeEvent* GetEvent(ttlib::cview name);
    NodeEvent* GetEvent(size_t index);

    auto GetPropertyCount() const { return m_properties.size(); }
    auto GetEventCount() const { return m_events.size(); }
    size_t GetInUseEventCount() const;

    Node* LocateAncestorType(GenEnum::GenType type) const noexcept;
    Node* FindParentForm() const noexcept;
    Node* GetForm() const noexcept { return FindParentForm(); }  // alias

    // Equivalent to AddChild(child); child->SetParent(this);
    // Returns false if child is not allowed for this node.
    bool Adopt(NodeSharedPtr child);

    bool AddChild(NodeSharedPtr node);
    bool AddChild(Node* node);
    bool AddChild(size_t index, NodeSharedPtr node);
    bool AddChild(size_t index, Node* node);

    // Returns the child's position or GetChildCount() in case of not finding it
    size_t GetChildPosition(Node* node);
    bool ChangeChildPosition(NodeSharedPtr node, size_t pos);

    void RemoveChild(NodeSharedPtr node);
    void RemoveChild(Node* node);
    void RemoveChild(size_t index);
    void RemoveAllChildren() { m_children.clear(); }

    NodeSharedPtr GetChildPtr(size_t index) { return m_children.at(index); }
    Node* GetChild(size_t index) const noexcept { return m_children.at(index).get(); }
    auto& GetChildNodePtrs() { return m_children; }

    auto GetChildCount() const { return m_children.size(); }

    bool IsChildAllowed(Node* child);
    bool IsChildAllowed(NodeDeclaration* child);
    bool IsChildAllowed(NodeSharedPtr child) { return IsChildAllowed(child.get()); }

    auto gen_type() const { return m_declaration->gen_type(); }

    // Returns the enum value for the name. Use DeclName() to get a char pointer.
    GenName gen_name() const { return m_declaration->gen_name(); }

    bool isType(GenType type) const noexcept { return (type == m_declaration->gen_type()); }
    bool isGen(GenName name) const noexcept { return (name == m_declaration->gen_name()); }
    bool isParent(GenName name) const noexcept
    {
        return (GetParent() ? name == GetParent()->m_declaration->gen_name() : false);
    }

    bool IsWidget() const noexcept { return isType(type_widget); }
    bool IsWizard() const noexcept { return isType(type_wizard); }
    bool IsMenuBar() const noexcept { return (isType(type_menubar_form) || isType(type_menubar)); }
    bool IsToolBar() const noexcept { return (isType(type_toolbar) || isType(type_toolbar_form)); }
    bool IsStatusBar() const noexcept { return isType(type_statusbar); }
    bool IsRibbonBar() const noexcept { return isType(type_ribbonbar); }

    bool IsForm() const noexcept;

    bool IsStaticBoxSizer() const noexcept
    {
        return (isGen(gen_wxStaticBoxSizer) || isGen(gen_StaticCheckboxBoxSizer) || isGen(gen_StaticRadioBtnBoxSizer));
    }
    bool IsSpacer() const noexcept { return isGen(gen_spacer); }

    bool IsSizer() const noexcept { return (isType(type_sizer) || isType(type_gbsizer)); }
    bool IsContainer() const noexcept
    {
        return (isType(type_container) || ttlib::contains(map_GenTypes[gen_type()], "book"));
    }

    // Returns true if access property == none or there is no access property
    bool IsLocal() const noexcept;

    auto GetNodeType() { return m_declaration->GetNodeType(); }
    auto GetGenerator() const { return m_declaration->GetGenerator(); }

    // Returns the value of the property "var_name" or "class_name"
    const ttlib::cstr& get_node_name() const;

    // Returns the value of the parent property "var_name" or "class_name"
    const ttlib::cstr& get_parent_name() const;

    // Finds the parent form and returns the value of the it's property "class_name"
    const ttlib::cstr& get_form_name() const;

    auto GetNodeDeclaration() { return m_declaration; }

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    bool HasValue(PropName name) const;

    // Returns true if the property exists
    bool HasProp(PropName name) const { return (m_prop_indices.find(name) != m_prop_indices.end()); }

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool isPropValue(PropName name, const char* value) const noexcept;

    // Returns true only if the property exists and it's value is equal to the parameter
    // value.
    bool isPropValue(PropName name, bool value) const noexcept;

    // Sets value only if the property exists, returns false if it doesn't exist.
    template <typename T>
    bool prop_set_value(PropName name, T value)
    {
        if (auto prop = get_prop_ptr(name); prop)
        {
            prop->set_value(value);
            return true;
        }
        else
        {
            return false;
        }
    }

    bool prop_as_bool(PropName name) const;

    // If type is option, id, or bitlist, this will convert that constant name to it's value
    // (see g_NodeCreator.GetConstantAsInt()). Otherwise, it calls atoi().
    int prop_as_int(PropName name) const;
    int prop_as_mockup(PropName name, std::string_view prefix) const;

    wxColour prop_as_wxColour(PropName name) const;
    wxFont prop_as_font(PropName name) const;
    wxPoint prop_as_wxPoint(PropName name) const;
    wxSize prop_as_wxSize(PropName name) const;
    wxAnimation prop_as_wxAnimation(PropName name) const;
    wxBitmap prop_as_wxBitmap(PropName name) const;
    wxArrayString prop_as_wxArrayString(PropName name) const;

    wxBitmapBundle prop_as_wxBitmapBundle(PropName name) const;

    FontProperty prop_as_font_prop(PropName name) const;
    double prop_as_double(PropName name) const;

    const ttlib::cstr& prop_as_string(PropName name) const;
    const ttlib::cstr& prop_as_constant(PropName name, std::string_view prefix);

    // Use with caution! This allows you to modify the property string directly.
    //
    // Returns nullptr if the property doesn't exist.
    ttlib::cstr* prop_as_raw_ptr(PropName name);

    const ttlib::cstr& prop_default_value(PropName name);

    // This will convert the string from UTF8 to UTF16 on Windows
    wxString prop_as_wxString(PropName name) const;

    wxSizerFlags GetSizerFlags() const;

    NodeSharedPtr GetSharedPtr() { return shared_from_this(); }

    std::vector<NodeProperty>& get_props_vector() { return m_properties; }

    NodeProperty* AddNodeProperty(PropDeclaration* info);
    NodeEvent* AddNodeEvent(const NodeEventInfo* info);
    void CreateDoc(pugi::xml_document& doc);

    // This creates an orphaned node -- it is the caller's responsibility to hook it up with
    // a parent.
    Node* CreateChildNode(GenName name);

    // Gets the current selected node and uses that to call CreateChildNode().
    Node* CreateNode(GenName name);

    // This is the preferred way to create a new node when requested by the user (tool, menu,
    // or dialog). Besides creating the node, some nodes will get special processing to
    // automatically create additional child nodes.
    bool CreateToolNode(GenName name);

    // This will modify the property and fire a EVT_NodePropChange event if the property
    // actually changed
    void ModifyProperty(PropName name, ttlib::cview value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(ttlib::cview name, ttlib::cview value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(ttlib::cview name, int value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(NodeProperty* prop, ttlib::cview value);

    // This will modify the property and fire a EVT_NodePropChange event
    void ModifyProperty(NodeProperty* prop, int value);

    // Both var_name and validator_variable properties are checked
    ttlib::cstr GetUniqueName(const ttlib::cstr& proposed_name);

    // Fix duplicate names in the current node and all of it's children
    void FixDuplicateNodeNames(Node* form = nullptr);

    bool FixDuplicateName();

    // Collects all unique var_name and validator_variable properties in the current form
    void CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node);

    int_t FindInsertionPos(Node* child) const;
    int_t FindInsertionPos(NodeSharedPtr child) const { return FindInsertionPos(child.get()); }

    // Currently only called in debug builds, but available for release builds should we need it
    size_t GetNodeSize() const;

    // This writes XML files in the 1.1 layout using attributes for properties
    void AddNodeToDoc(pugi::xml_node& object);

    void CalcNodeHash(size_t& hash) const;

    int_t GetAllowableChildren(GenType child_gen_type) const { return m_declaration->GetAllowableChildren(child_gen_type); }

    // Collect a vector of pointers to all children having the specified property with a
    // non-empty value.
    std::vector<NodeProperty*> FindAllChildProperties(PropName name);

protected:
    void PostProcessBook(Node* book_node);
    void PostProcessPage(Node* page_node);

    void FindAllChildProperties(std::vector<NodeProperty*>& list, PropName name);

    // wxPanel only, not FormPanel
    void PostProcessPanel(Node* panel_node);

private:
    NodeSharedPtr m_parent;

    // Properties and events are added when the node is created, and then never changed for the life of the node --
    // only the value of the property or event is changed.

    // The vector makes it possible to iterate through the properties in the order they were created in the XML file. The
    // map gives us a fast lookup into the vector.
    std::vector<NodeProperty> m_properties;
    std::map<PropName, size_t> m_prop_indices;

    std::vector<NodeEvent> m_events;
    std::unordered_map<std::string, size_t> m_event_map;

    std::vector<NodeSharedPtr> m_children;
    NodeDeclaration* m_declaration;
};

// Same as wxGetApp() only this returns a reference to the project node
Node& wxGetProject();
