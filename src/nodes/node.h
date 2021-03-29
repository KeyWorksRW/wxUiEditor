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

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/sizer.h>   // provide wxSizer class for layout

#include "../pugixml/pugixml.hpp"

#include "font_prop.h"   // FontProperty class
#include "node_decl.h"   // NodeDeclaration class
#include "node_event.h"  // NodeEvent and NodeEventInfo classes
#include "node_prop.h"   // NodeProperty class
#include "node_types.h"  // NodeType -- Class for storing component types and allowable child count
#include "prop_names.h"  // Property names

class Node;
using NodeSharedPtr = std::shared_ptr<Node>;
using ChildNodePtrs = std::vector<NodeSharedPtr>;

class Node : public std::enable_shared_from_this<Node>
{
public:
    Node(NodeDeclaration* info);
    ~Node();

    const ttlib::cstr& GetClassName() const { return m_info->GetClassName(); }

    NodeSharedPtr GetParentPtr() { return m_parent; }
    Node* GetParent() { return m_parent.get(); }

    void SetParent(NodeSharedPtr parent) { m_parent = parent; }
    void SetParent(Node* parent) { m_parent = parent->GetSharedPtr(); }

    NodeProperty* get_prop_ptr(ttlib::cview name);
    ttlib::cstr* get_value_ptr(ttlib::cview name);

    NodeEvent* GetEvent(ttlib::cview name);
    NodeEvent* GetEvent(size_t index);

    size_t GetPropertyCount() { return m_properties.size(); }
    size_t GetEventCount() { return m_events.size(); }
    size_t GetInUseEventCount();

    NodeSharedPtr FindNearAncestorPtr(const std::string& type);
    NodeSharedPtr FindParentFormPtr();

    Node* FindNearAncestor(const std::string& type);
    Node* FindNearAncestorByBaseClass(const std::string& type);
    Node* FindParentForm();

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

    // Note that this returns a copy rather than a const ref
    NodeSharedPtr GetChildPtr(size_t index) { return m_children.at(index); }

    Node* GetChild(size_t index) { return m_children.at(index).get(); }

    size_t GetChildCount() { return m_children.size(); }

    ChildNodePtrs& GetChildNodePtrs() { return m_children; }

    bool IsChildAllowed(Node* child);
    bool IsChildAllowed(NodeSharedPtr child) { return IsChildAllowed(child.get()); }

    bool IsChildType(size_t index, ttlib::cview type);

    bool IsWidget() { return (GetNodeTypeName() == "widget"); }
    bool IsWizard() { return (GetNodeTypeName() == "wizard"); }
    bool IsMenuBar() { return (GetNodeTypeName() == "menubar_form" || GetNodeTypeName() == "menubar"); }
    bool IsToolBar() { return (GetNodeTypeName() == "toolbar" || GetNodeTypeName() == "toolbar_form"); }
    bool IsStatusBar() { return (GetNodeTypeName() == "statusbar"); }
    bool IsRibbonBar() { return (GetNodeTypeName() == "ribbonbar"); }

    // This does not include MenuBar, ToolBar, StatusBar or Wizard
    bool IsForm()
    {
        return (GetNodeTypeName() == "form" || GetNodeTypeName() == "menubar_form" || GetNodeTypeName() == "toolbar_form" ||
                GetNodeTypeName() == "wizard");
    }
    bool IsStaticBoxSizer()
    {
        return (GetClassName() == "wxStaticBoxSizer" || GetClassName() == "StaticCheckboxBoxSizer" ||
                GetClassName() == "StaticRadioBtnBoxSizer");
    }
    bool IsSpacer() { return (GetClassName() == "spacer"); }

    bool IsSizer() { return (GetNodeTypeName() == "sizer" || GetNodeTypeName() == "gbsizer"); }
    bool IsContainer() { return (GetNodeTypeName() == "container" || GetNodeTypeName().contains("book")); }

    // Returns true if access property == none or there is no access property
    bool IsLocal();

    const ttlib::cstr& GetNodeTypeName() { return m_info->GetNodeTypeName(); }
    NodeType* GetNodeType() { return m_info->GetNodeType(); }
    BaseGenerator* GetGenerator() const { return m_info->GetGenerator(); }

    // Returns the value of the property "var_name" or "class_name"
    const ttlib::cstr& get_node_name();

    // Returns the value of the parent property "var_name" or "class_name"
    const ttlib::cstr& get_parent_name();

    // Finds the parent form and returns the value of the it's property "class_name"
    const ttlib::cstr& get_form_name();

    NodeDeclaration* GetNodeDeclaration() { return m_info; }

    bool HasValue(ttlib::cview name);

    // Returns true if the property exists, has a value (!= wxDefaultSize, !=
    // wxDefaultPosition, or non-sepcified bitmap)
    bool prop_has_value(ttlib::cview name);

    bool prop_as_bool(ttlib::cview name);
    int prop_as_int(ttlib::cview name);

    wxColour prop_as_wxColour(ttlib::cview name);
    wxFont prop_as_font(ttlib::cview name);
    wxPoint prop_as_wxPoint(ttlib::cview name);
    wxSize prop_as_wxSize(ttlib::cview name);
    wxBitmap prop_as_wxBitmap(ttlib::cview name);
    wxArrayString prop_as_wxArrayString(ttlib::cview name);

    FontProperty prop_as_font_prop(ttlib::cview name);
    double GetPropertyAsFloat(ttlib::cview name);

    // This will convert the string from UTF8 to UTF16 on Windows
    wxString GetPropertyAsString(ttlib::cview name);

    const ttlib::cstr& prop_as_string(ttlib::cview name);
    wxString prop_as_wxString(ttlib::cview name) { return GetPropertyAsString(name); }

    wxSizerFlags GetSizerFlags();

    NodeSharedPtr GetSharedPtr() { return shared_from_this(); }

    std::vector<NodeProperty>& get_props_vector() { return m_properties; }

    NodeProperty* AddNodeProperty(PropertyInfo* info);
    NodeEvent* AddNodeEvent(const NodeEventInfo* info);
    void CreateDoc(pugi::xml_document& doc);

    // This creates an orphaned node -- it is the caller's responsibility to hook it up with
    // a parent.
    Node* CreateChildNode(ttlib::cview name);

    Node* CreateNode(ttlib::cview name);
    bool CreateToolNode(const ttlib::cstr& name);

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

    bool FixDuplicateName(bool is_validator = false);

    // Fix duplicate names in the current node and all of it's children
    void FixPastedNames();

    // Collects all unique var_name and validator_variable properties in the current form
    void CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node);

    int_t FindInsertionPos(Node* child);
    int_t FindInsertionPos(NodeSharedPtr child) { return FindInsertionPos(child.get()); }

protected:
    // This writes XML files in the 1.1 layout using attributes for properties
    void AddNodeToDoc(pugi::xml_node& object);

private:
    NodeSharedPtr m_parent;

    // Properties and events are added when the node is created, and then never changed for the life of the node --
    // only the value of the property or event is changed.

    // The vector makes it possible to iterate through the properties in the order they were created in the XML file. The
    // unordered map gives us a fast lookup into the vector.
    std::vector<NodeProperty> m_properties;
    std::unordered_map<std::string, size_t> m_prop_map;

    std::vector<NodeEvent> m_events;
    std::unordered_map<std::string, size_t> m_event_map;

    ChildNodePtrs m_children;
    NodeDeclaration* m_info;
};

// Same as wxGetApp() only this returns a reference to the project node
Node& wxGetProject();
