/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "node_creator.h"

#include "gen_enums.h"    // Enumerations for nodes
#include "node.h"         // Node class
#include "prop_decl.h"    // PropChildDeclaration and PropDeclaration classes

NodeCreator& NodeCreation = NodeCreator::getInstance();

using namespace GenEnum;

NodeDeclaration* NodeCreator::getNodeDeclaration(tt_string_view className)
{
    if (auto result = rmap_GenNames.find(className); result != rmap_GenNames.end())
    {
        return m_a_declarations[result->second];
    }

    // Failure can happen for a new category that didn't get added to the interface section
    // of map_GenNames in gen_enums.cpp

    FAIL_MSG(tt_string() << "Attempt to get non-existant node declaration for " << className);
    return nullptr;
}

// clang-format off
inline const auto lstBitmapoProps = {
    prop_bitmap,
    prop_current,
    prop_disabled_bmp,
    prop_focus_bmp,
    prop_inactive_bitmap,
    prop_pressed_bmp,
};
// clang-format on

NodeSharedPtr NodeCreator::newNode(NodeDeclaration* node_decl)
{
    auto node = std::make_shared<Node>(node_decl);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = node_decl->GetBaseClassCount();

    size_t base = 0;
    for (auto class_info = node_decl; class_info; class_info = node_decl->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->getPropertyCount(); ++index)
        {
            auto prop_declaration = class_info->getPropDeclaration(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_declaration->getDefaultValue();
            if (base > 0)
            {
                auto result = node_decl->GetOverRideDefValue(prop_declaration->get_name());
                if (result)
                    defaultValue = result.value();
            }

            auto prop = node->addNodeProperty(prop_declaration);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->getEventCount(); ++index)
        {
            node->addNodeEvent(class_info->getEventInfo(index));
        }

        if (base >= node_info_base_count)
            break;
    }

    return node;
}

size_t NodeCreator::countChildrenWithSameType(Node* parent, GenType type)
{
    size_t count = 0;
    for (const auto& child: parent->getChildNodePtrs())
    {
        if (type == child->getGenType())
            ++count;

        // treat type-sizer and type_gbsizer as the same since forms and contains can only have one of them as the top level
        // sizer.

        else if (type == type_sizer && child->getGenType() == type_gbsizer)
            ++count;
        else if (type == type_gbsizer && child->getGenType() == type_sizer)
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
NodeSharedPtr NodeCreator::createNode(GenName name, Node* parent)
{
    NodeSharedPtr node;
    NodeDeclaration* node_decl;

    ASSERT(name != gen_unknown);
    if (name == gen_unknown)
        return node;

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
        return newNode(node_decl);

    // This happens when importing wxFormBuilder projects
    if (isOldHostType(node_decl->declName()))
        return newNode(node_decl);

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
        auto grand_parent = parent->getParent();
        if (grand_parent->isGen(gen_wxToolBar) && node_decl->isType(type_menu))
            return NodeSharedPtr();
    }
    else if (name == gen_BookPage && parent->isType(type_bookpage))
    {
        auto grand_parent = parent->getParent();
        if (!grand_parent || !grand_parent->isGen(gen_wxTreebook))
        {
            return NodeSharedPtr();
        }
    }

    auto max_children = parent->getAllowableChildren(node_decl->getGenType());

    if (max_children == child_count::infinite)
    {
        node = newNode(node_decl);
        if (name == gen_VerticalBoxSizer)
        {
            node->set_value(prop_orientation, "wxVERTICAL");
        }
    }
    else if (max_children != child_count::none)
    {
        if (node_decl->isType(type_sizer))
        {
            auto count = countChildrenWithSameType(parent, node_decl->getGenType());
            if (count < (to_size_t) max_children)
            {
                node = newNode(node_decl);
                if (name == gen_VerticalBoxSizer)
                {
                    node->set_value(prop_orientation, "wxVERTICAL");
                }
            }
        }
        else if (node_decl->isType(type_gbsizer))
        {
            auto count = countChildrenWithSameType(parent, node_decl->getGenType());
            if (count < (to_size_t) max_children)
            {
                node = newNode(node_decl);
            }
        }
        else if (parent->isGen(gen_wxSplitterWindow))
        {
            // for splitters, we only care if the type is allowed, and if the splitter only has one child so far.
            if (parent->getChildCount() < 2)
                node = newNode(node_decl);
        }
        else
        {
            auto count = countChildrenWithSameType(parent, node_decl->getGenType());
            if (count < (to_size_t) max_children)
            {
                node = newNode(node_decl);
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
NodeSharedPtr NodeCreator::createNode(tt_string_view name, Node* parent)
{
    if (auto result = rmap_GenNames.find(name); result != rmap_GenNames.end())
    {
        return createNode(result->second, parent);
    }

    FAIL_MSG(tt_string() << "No component definition for " << name);
    return {};
}

NodeSharedPtr NodeCreator::makeCopy(Node* node, Node* parent)
{
    ASSERT(node);
    NodeSharedPtr copyObj;

    // Sometimes we need to copy a similar node to a new node using the parent as the guide.
    if (parent)
    {
        if (node->isGen(gen_tool) && (parent->isGen(gen_wxAuiToolBar) || parent->isGen(gen_AuiToolBar)))
            copyObj = newNode(gen_auitool);
        else if (node->isGen(gen_auitool) && (parent->isGen(gen_wxToolBar) || parent->isGen(gen_ToolBar)))
            copyObj = newNode(gen_tool);
    }

    if (!copyObj)
    {
        copyObj = newNode(node->getNodeDeclaration());
    }

    ASSERT(copyObj);

    for (auto& iter: node->getPropsVector())
    {
        if (auto copyProp = copyObj->getPropPtr(iter.get_name()); copyProp)
        {
            copyProp->set_value(iter.as_string());
        }
    }

    copyObj->copyEventsFrom(node);

    // It will be rare, but sometimes a user may want to copy a form such as FormPanel and paste it into a sizer or a book.
    // In that case, we need to create the non-form version of the control.
    if (parent && !parent->isGen(gen_Project) && node->isForm())
    {
        NodeSharedPtr child_object;
        if (node->isGen(gen_ToolBar))
        {
            child_object = createNode(gen_wxToolBar, parent);
            // REVIEW: [Randalphwa - 10-06-2022] This will fail if the parent is a wxFrame and it already has a toolbar.
            // Should we let the user know?
        }
        else if (node->isGen(gen_MenuBar))
        {
            child_object = createNode(gen_wxMenuBar, parent);
        }
        else if (node->isGen(gen_RibbonBar))
        {
            child_object = createNode(gen_wxRibbonBar, parent);
        }
        else if (node->isGen(gen_PanelForm))
        {
            if (parent->isType(type_choicebook) || parent->isType(type_listbook) || parent->isType(type_notebook) ||
                parent->isType(type_simplebook))
            {
                child_object = createNode(gen_BookPage, parent);
            }
            else
            {
                child_object = createNode(gen_wxPanel, parent);
            }
        }

        if (child_object)
        {
            for (auto& iter: node->getPropsVector())
            {
                auto copyProp = child_object->getPropPtr(iter.get_name());
                // Note that due to the conversion, not all properties will actually exist
                if (copyProp)
                    copyProp->set_value(iter.as_string());
            }
            child_object->copyEventsFrom(node);
            copyObj = child_object;
        }
    }

#if 0
    auto count = node->getEventCount();
    for (size_t i = 0; i < count; i++)
    {
        auto event = node->getEvent(i);
        auto copyEvent = copyObj->getEvent(event->get_name());
        ASSERT(copyEvent);
        if (copyEvent)
            copyEvent->set_value(event->get_value());
    }
#endif

    for (auto& child: node->getChildNodePtrs())
    {
        if (auto childCopy = makeCopy(child.get()); childCopy)
        {
            copyObj->adoptChild(childCopy);
        }
    }

    return copyObj;
}

int NodeCreator::getConstantAsInt(const std::string& name, int defValue) const
{
    if (auto iter = m_map_constants.find(name); iter != m_map_constants.end())
        return iter->second;
    else
        return defValue;
}
