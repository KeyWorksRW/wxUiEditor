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
#include "prop_decl.h"  // PropChildDeclaration and PropDeclaration classes

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
NodeSharedPtr NodeCreator::NewNode(NodeDeclaration* node_decl)
{
    auto node = std::make_shared<Node>(node_decl);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = node_decl->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = node_decl; class_info; class_info = node_decl->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->GetPropertyCount(); ++index)
        {
            auto prop_declaration = class_info->GetPropDeclaration(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_declaration->GetDefaultValue();
            if (base > 0)
            {
                auto defaultValueTemp =
                    node_decl->GetBaseClassDefaultPropertyValue(base - 1, prop_declaration->DeclName().c_str());
                if (!defaultValueTemp.empty())
                {
                    defaultValue = defaultValueTemp;
                }
            }

            auto prop = node->AddNodeProperty(prop_declaration);
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

    * This will return nullptr if the parent doesn't allow this child type, or the parent already has the maximum number of
    * children allowed. E.g., containers can only have one child, splitters can only have two, and sizers can have an
    * unlimited number.

*/

// The parent parameter is used to determine if the parent allows this type of child, and if so how many of those
// children are allowed.
NodeSharedPtr NodeCreator::CreateNode(ttlib::cview classname, Node* parent)
{
    NodeSharedPtr node;
    NodeDeclaration* node_decl;

    // This is a way for a ribbon panel button to indicate a wxBoxSizer with vertical orientation
    if (classname.is_sameas("VerticalBoxSizer"))
        node_decl = GetNodeDeclaration("wxBoxSizer");
    else
        node_decl = GetNodeDeclaration(classname);

    if (!node_decl)
    {
        FAIL_MSG(ttlib::cstr() << "No component definition for " << classname);
        throw std::runtime_error("Internal error: missing component definition");
    }

    if (!parent)
        return NewNode(node_decl);

    // This happens when importing wxFormBuilder and old wxUiEditor projects
    if (IsOldHostType(classname))
        return NewNode(node_decl);

    auto comp_type = node_decl->GetNodeType();

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

    // Currently we don't support aui, but once we do we'll need to pass the paren't current setting to GetAllowableChildren
    bool aui = false;

    auto max_children = GetAllowableChildren(parent, comp_type->gen_type(), aui);

    if (max_children == child_count::infinite)
    {
        node = NewNode(node_decl);
        if (classname.is_sameas("VerticalBoxSizer"))
        {
            node->prop_set_value(prop_orientation, "wxVERTICAL");
        }
    }
    else if (max_children != child_count::none)
    {
        if (comp_type == GetNodeType(type_sizer))
        {
            node = NewNode(node_decl);
            if (classname.is_sameas("VerticalBoxSizer"))
            {
                node->prop_set_value(prop_orientation, "wxVERTICAL");
            }
        }
        else if (comp_type == GetNodeType(type_gbsizer))
        {
            node = NewNode(node_decl);
        }
        else
        {
            auto count = CountChildrenWithSameType(parent, comp_type);
            // REVIEW: [KeyWorks - 04-11-2021] Does this actually happen? And if it does, we need to let the user know. Note
            // that once aui is supported, this may start happening since aui typically allows one non-sizer child.
            ASSERT_MSG(count < (size_t) max_children,
                       "Parent allows one of this child type, a second of the same type is not allowed");
            if (count < (size_t) max_children)
                node = NewNode(node_decl);
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
        auto copyProp = copyObj->get_prop_ptr(iter.get_name());
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
    auto node_decl = node->GetNodeDeclaration();

    // Caution: Do NOT place spaces around the | that combines flags. Other parts of the codebase rely on there being no
    // spaces...

    if (node_decl->isGen(gen_wxStdDialogButtonSizer) || node_decl->isGen(gen_wxStaticLine))
    {
        node->prop_set_value(prop_borders, "wxALL");
        node->prop_set_value(prop_flags, "wxEXPAND");
        return;
    }

    if (node->IsSizer() || node->isType(type_splitter) || node_decl->isGen(gen_spacer))
    {
        node->prop_set_value(prop_proportion, "1");
        node->prop_set_value(prop_flags, "wxEXPAND");
    }
    else if (node_decl->isGen(gen_wxToolBar))
    {
        node->prop_set_value(prop_flags, "wxEXPAND");
    }
    else if (node->isType(type_widget) || node->isType(type_statusbar))
    {
        node->prop_set_value(prop_proportion, "0");
        node->prop_set_value(prop_borders, "wxALL");
    }
    else if (node->isType(type_notebook) || node->isType(type_listbook) || node->isType(type_simplebook) ||
             node->isType(type_choicebook) || node->isType(type_auinotebook) || node->isType(type_treelistctrl) ||
             node->isType(type_expanded_widget) || node->isType(type_container))
    {
        node->prop_set_value(prop_proportion, "1");
        node->prop_set_value(prop_borders, "wxALL");
        node->prop_set_value(prop_flags, "wxEXPAND");
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
