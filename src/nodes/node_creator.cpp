/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "ttcstr.h"      // cstr -- std::string with additional methods
#include "ttlibspace.h"  // ttlib namespace functions and declarations

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

NodeDeclaration* NodeCreator::GetNodeDeclaration(ttlib::sview className)
{
    if (auto result = rmap_GenNames.find(className); result != rmap_GenNames.end())
    {
        return m_a_declarations[result->second];
    }

    FAIL_MSG(ttlib::cstr() << "Attempt to get non-existant node declaration for " << className);
    return nullptr;
}

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
                auto result = node_decl->GetOverRideDefValue(prop_declaration->get_name());
                if (result)
                    defaultValue = result.value();
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

size_t NodeCreator::CountChildrenWithSameType(Node* parent, GenType type)
{
    size_t count = 0;
    for (const auto& child: parent->GetChildNodePtrs())
    {
        if (type == child->gen_type())
            ++count;

        // treat type-sizer and type_gbsizer as the same since forms and contains can only have one of them as the top level
        // sizer.

        else if (type == type_sizer && child->gen_type() == type_gbsizer)
            ++count;
        else if (type == type_gbsizer && child->gen_type() == type_sizer)
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
NodeSharedPtr NodeCreator::CreateNode(GenName name, Node* parent)
{
    NodeSharedPtr node;
    NodeDeclaration* node_decl;

    // This is a way for a ribbon panel button to indicate a wxBoxSizer with vertical orientation
    if (name == gen_VerticalBoxSizer)
        node_decl = m_a_declarations[gen_wxBoxSizer];
    else
        node_decl = m_a_declarations[name];

    if (!node_decl)
    {
        // Unless the toolbar is a child of a wxAui frame window, there's little to no difference between a wxAuiToolBar and
        // a wxToolBar. Checking it here allows us to automatically convert imported projects, and then if we ever do decide
        // to support wxAuiToolBar, imports will immediately switch without having to touch the import code.

        if (name == gen_wxAuiToolBar)
            node_decl = m_a_declarations[gen_wxToolBar];
        else
            return NodeSharedPtr();
    }

    if (!parent)
        return NewNode(node_decl);

    // This happens when importing wxFormBuilder projects
    if (IsOldHostType(node_decl->DeclName()))
        return NewNode(node_decl);

    // Check for widgets which can ONLY have a frame for a parent.
    if (node_decl->isType(type_statusbar) || node_decl->isType(type_menubar) || node_decl->isType(type_toolbar))
    {
        if (parent->isType(type_form) && !parent->isGen(gen_wxFrame))
        {
            return NodeSharedPtr();
        }
    }
    else if (parent->isType(type_tool))
    {
        auto grand_parent = parent->GetParent();
        if (grand_parent->isGen(gen_wxToolBar) && node_decl->isType(type_menu))
            return NodeSharedPtr();
    }
    else if (name == gen_BookPage && parent->isType(type_bookpage))
    {
        auto grand_parent = parent->GetParent();
        if (!grand_parent || !grand_parent->isGen(gen_wxTreebook))
        {
            return NodeSharedPtr();
        }
    }

    auto max_children = parent->GetAllowableChildren(node_decl->gen_type());

    if (max_children == child_count::infinite)
    {
        node = NewNode(node_decl);
        if (name == gen_VerticalBoxSizer)
        {
            node->prop_set_value(prop_orientation, "wxVERTICAL");
        }
    }
    else if (max_children != child_count::none)
    {
        if (node_decl->isType(type_sizer))
        {
            auto count = CountChildrenWithSameType(parent, node_decl->gen_type());
            if (count < (to_size_t) max_children)
            {
                node = NewNode(node_decl);
                if (name == gen_VerticalBoxSizer)
                {
                    node->prop_set_value(prop_orientation, "wxVERTICAL");
                }
            }
        }
        else if (node_decl->isType(type_gbsizer))
        {
            auto count = CountChildrenWithSameType(parent, node_decl->gen_type());
            if (count < (to_size_t) max_children)
            {
                node = NewNode(node_decl);
            }
        }
        else if (parent->isGen(gen_wxSplitterWindow))
        {
            // for splitters, we only care if the type is allowed, and if the splitter only has one child so far.
            if (parent->GetChildCount() < 2)
                node = NewNode(node_decl);
        }
        else
        {
            auto count = CountChildrenWithSameType(parent, node_decl->gen_type());
            if (count < (to_size_t) max_children)
            {
                node = NewNode(node_decl);
            }
        }
    }
    else
    {
        // The parent doesn't allow this child type. Since node is empty at this point, we simply fall through and return
        // it. It's the caller's responsibility to try to find a proper parent.
    }

    return node;
}

// Called when the GenName isn't availalble
NodeSharedPtr NodeCreator::CreateNode(ttlib::sview name, Node* parent)
{
    if (auto result = rmap_GenNames.find(name); result != rmap_GenNames.end())
    {
        return CreateNode(result->second, parent);
    }

    FAIL_MSG(ttlib::cstr() << "No component definition for " << name);
    return {};
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

        if (copyProp)
            copyProp->set_value(iter.as_string());
    }

    copyObj->CopyEventsFrom(node);

#if 0
    auto count = node->GetEventCount();
    for (size_t i = 0; i < count; i++)
    {
        auto event = node->GetEvent(i);
        auto copyEvent = copyObj->GetEvent(event->get_name());
        ASSERT(copyEvent);
        if (copyEvent)
            copyEvent->set_value(event->get_value());
    }
#endif

    for (auto& child: node->GetChildNodePtrs())
    {
        if (auto childCopy = MakeCopy(child.get()); childCopy)
        {
            copyObj->Adopt(childCopy);
        }
    }

    return copyObj;
}

int NodeCreator::GetConstantAsInt(const std::string& name, int defValue) const
{
    if (auto iter = m_map_constants.find(name); iter != m_map_constants.end())
        return iter->second;
    else
        return defValue;
}
