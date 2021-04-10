/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <ttcstr.h>
#include <ttlibspace.h>

#include "node_creator.h"

#include "gen_enums.h"  // Enumerations for nodes
#include "node.h"       // Node class
#include "prop_info.h"  // PropDefinition and PropertyInfo classes

NodeCreator g_NodeCreator;

using namespace GenEnum;

NodeCreator::~NodeCreator()
{
    for (auto& iter: m_a_declarations)
    {
        delete iter;
    }
}

NodeDeclaration* NodeCreator::GetNodeDeclaration(ttlib::cview className)
{
    return m_a_declarations[static_cast<size_t>(rmap_GenNames[className.c_str()])];
}

// This will add all properties and events, including any base interface classes such as wxWindow, sizeritem, etc.
NodeSharedPtr NodeCreator::NewNode(NodeDeclaration* declaration)
{
    auto node = std::make_shared<Node>(declaration);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = declaration->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = declaration; class_info; class_info = declaration->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->GetPropertyCount(); ++index)
        {
            auto prop_info = class_info->GetPropertyInfo(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_info->GetDefaultValue();
            if (base > 0)
            {
                auto defaultValueTemp =
                    declaration->GetBaseClassDefaultPropertyValue(base - 1, prop_info->GetName().c_str());
                if (!defaultValueTemp.empty())
                {
                    defaultValue = defaultValueTemp;
                }
            }

            auto prop = node->AddNodeProperty(prop_info);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->GetEventCount(); ++index)
        {
            node->AddNodeEvent(class_info->GetEventInfo(index));
        }

        if (base >= node_info_base_count)
            break;
    }

    return node;
}

size_t NodeCreator::CountChildrenWithSameType(Node* parent, NodeType* type)
{
    size_t count = 0;
    size_t numChildren = parent->GetChildCount();
    for (size_t i = 0; i < numChildren; ++i)
    {
        if (type == parent->GetChild(i)->GetNodeDeclaration()->GetNodeType())
            ++count;
    }

    return count;
}

/*

    * This will return NULL if the parent doesn't allow this child type, or the parent already has the maximum number of
    * children allowed. E.g., containers can only have one child, splitters can only have two, and sizers can have an
    * unlimited number.

*/

// The parent parameter is used to determine if the parent allows this type of child, and if so how many of those
// children are allowed.
NodeSharedPtr NodeCreator::CreateNode(ttlib::cview classname, Node* parent)
{
    NodeSharedPtr node;
    NodeDeclaration* declaration;

    // This is a way for a ribbon panel button to indicate a wxBoxSizer with vertical orientation
    if (classname.is_sameas("VerticalBoxSizer"))
        declaration = GetNodeDeclaration("wxBoxSizer");
    else
        declaration = GetNodeDeclaration(classname);

    if (!declaration)
    {
        FAIL_MSG(ttlib::cstr() << "No component definition for " << classname);
        throw std::runtime_error("Internal error: missing component definition");
    }

    if (!parent)
        return NewNode(declaration);

    // This happens when importing wxFormBuilder and old wxUiEditor projects
    if (IsOldHostType(classname))
        return NewNode(declaration);

    bool aui = false;
    if (parent->isType(type_form))
    {
        aui = parent->prop_as_int("aui_managed") != 0;
    }

    auto comp_type = declaration->GetNodeType();

    // Check for widgets which can ONLY have a frame for a parent.
    if (comp_type->isType(type_statusbar) || comp_type->isType(type_menubar) || comp_type->isType(type_ribbonbar) ||
        comp_type->isType(type_toolbar))
    {
        if (parent->isType(type_form) && !parent->isGen(gen_wxFrame))
        {
            return NodeSharedPtr();
        }
    }
    else if (parent->isType(type_tool))
    {
        auto grand_parent = parent->GetParent();
        if (grand_parent->isGen(gen_wxToolBar) && comp_type->isType(type_menu))
            return NodeSharedPtr();
    }

    auto max_children = GetAllowableChildren(parent, comp_type->gen_type(), aui);

    if (max_children == child_count::infinite)
    {
        node = NewNode(declaration);
        if (classname.is_sameas("VerticalBoxSizer"))
            node->get_prop_ptr(txt_orientation)->set_value("wxVERTICAL");
    }
    else if (max_children != child_count::none)
    {
        if (comp_type == GetNodeType(type_sizer))
        {
            node = NewNode(declaration);
            if (classname.is_sameas("VerticalBoxSizer"))
                node->get_prop_ptr(txt_orientation)->set_value("wxVERTICAL");
        }
        else if (comp_type == GetNodeType(type_gbsizer))
        {
            node = NewNode(declaration);
        }
        else
        {
            auto count = CountChildrenWithSameType(parent, comp_type);
            ASSERT_MSG(count < (size_t) max_children,
                       "Parent allows one of this child type, a second of the same type is not allowed");
            if (count < (size_t) max_children)
                node = NewNode(declaration);
        }
    }
    else
    {
        // The parent doesn't allow this child type. Since node is empty at this point, we simply fall through and return
        // it. It's the caller's responsibility to try to find a proper parent.
    }

    return node;
}

NodeSharedPtr NodeCreator::MakeCopy(Node* node)
{
    ASSERT(node);

    auto objInfo = node->GetNodeDeclaration();

    auto copyObj = NewNode(objInfo);
    ASSERT(copyObj);

    for (auto& iter: node->get_props_vector())
    {
        auto copyProp = copyObj->get_prop_ptr(iter.GetPropName());
        ASSERT(copyProp);

        copyProp->set_value(iter.as_string());
    }

    auto count = node->GetEventCount();
    for (size_t i = 0; i < count; i++)
    {
        auto event = node->GetEvent(i);
        auto copyEvent = copyObj->GetEvent(event->get_name());
        ASSERT(copyEvent)
        copyEvent->set_value(event->get_value());
    }

    count = node->GetChildCount();
    for (size_t i = 0; i < count; i++)
    {
        auto childCopy = MakeCopy(node->GetChild(i));
        ASSERT(childCopy)
        copyObj->AddChild(childCopy);
        childCopy->SetParent(copyObj);
    }

    return copyObj;
}

// TODO: [KeyWorks - 11-21-2020] Once a new node is created and placed in it's heirachy, the code that created the
// node should call this to setup default sizer settings.
void NodeCreator::SetDefaultLayoutProperties(Node* node)
{
    auto declaration = node->GetNodeDeclaration();

    // Caution: Do NOT place spaces around the | that combines flags. Other parts of the codebase rely on there being no
    // spaces...

    if (declaration->GetClassName() == "wxStdDialogButtonSizer")
    {
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
        return;
    }

    auto node_type = node->GetNodeTypeName();
    auto proportion = node->get_prop_ptr(txt_proportion);

    if (declaration->GetClassName() == "wxStaticLine" || declaration->GetClassName() == "wxStdDialogButtonSizer")
    {
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (node->IsSizer() || node_type == "splitter" || declaration->GetClassName() == "spacer")
    {
        if (proportion)
        {
            proportion->set_value("1");
        }
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (declaration->GetClassName() == "wxToolBar")
    {
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
    else if (node_type == "widget" || node_type == "statusbar")
    {
        if (proportion)
        {
            proportion->set_value("0");
        }
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
    }
    else if (node_type == "notebook" || node_type == "listbook" || node_type == "simplebook" || node_type == "choicebook" ||
             node_type == "auinotebook" || node_type == "treelistctrl" || node_type == "expanded_widget" ||
             node_type == "container")
    {
        if (proportion)
        {
            proportion->set_value("1");
        }
        node->get_prop_ptr(txt_borders)->set_value("wxALL");
        node->get_prop_ptr(txt_flags)->set_value("wxEXPAND");
    }
}

int_t NodeCreator::GetAllowableChildren(Node* parent, GenType child_class_type, bool is_aui_parent) const
{
    return parent->GetNodeDeclaration()->GetNodeType()->GetAllowableChildren(child_class_type, is_aui_parent);
}

int NodeCreator::GetConstantAsInt(const std::string& name, int defValue)
{
    if (auto iter = m_map_constants.find(name); iter != m_map_constants.end())
        return iter->second;
    else
        return defValue;
}
