/////////////////////////////////////////////////////////////////////////////
// Purpose:   Class used to create nodes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>

#include "node_creator.h"

#include "base_generator.h"   // BaseGenerator -- Base Generator class
#include "gen_enums.h"        // Enumerations for nodes
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler class
#include "prop_decl.h"        // PropChildDeclaration and PropDeclaration classes

NodeCreator& NodeCreation = NodeCreator::get_Instance();  // NOLINT (cppcheck-suppress)

using namespace GenEnum;

auto NodeCreator::get_NodeDeclaration(std::string_view class_name) -> NodeDeclaration*
{
    if (auto result = rmap_GenNames.find(class_name); result != rmap_GenNames.end())
    {
        return m_a_declarations.at(result->second);
    }

    // Failure can happen for a new category that didn't get added to the interface section
    // of map_GenNames in gen_enums.cpp

    FAIL_MSG(std::format("Attempt to get non-existant node declaration for {}", class_name));
    return nullptr;
}

auto NodeCreator::NewNode(NodeDeclaration* node_decl) -> NodeSharedPtr
{
    auto node = std::make_shared<Node>(node_decl);

    // Calling GetBaseClassCount() is expensive, so do it once and store the result
    auto node_info_base_count = node_decl->GetBaseClassCount();

    size_t base = 0;
    for (auto* class_info = node_decl; class_info; class_info = node_decl->GetBaseClass(base++))
    {
        for (size_t index = 0; index < class_info->get_PropertyCount(); ++index)
        {
            auto* prop_declaration = class_info->get_PropDeclaration(index);

            // Set the default value, either from the property info, or an override from this class
            auto defaultValue = prop_declaration->getDefaultValue();
            if (base > 0)
            {
                auto result = node_decl->GetOverRideDefValue(prop_declaration->get_name());
                if (result)
                {
                    defaultValue = result.value();
                }
            }

            auto* prop = node->AddNodeProperty(prop_declaration);
            prop->set_value(defaultValue);
        }

        for (size_t index = 0; index < class_info->get_EventCount(); ++index)
        {
            node->AddNodeEvent(class_info->get_EventInfo(index));
        }

        if (base >= node_info_base_count)
        {
            break;
        }
    }

    return node;
}

auto NodeCreator::CountChildrenWithSameType(Node* parent, GenType type) -> size_t
{
    size_t count = 0;
    for (const auto& child: parent->get_ChildNodePtrs())
    {
        // treat type-sizer and type_gbsizer as the same since forms and contains can only have
        // one of them as the top level sizer.
        if (type == child->get_GenType() ||
            (type == type_sizer && child->get_GenType() == type_gbsizer) ||
            (type == type_gbsizer && child->get_GenType() == type_sizer))
        {
            ++count;
        }
    }

    return count;
}

/*

    This will return nullptr if the parent doesn't allow this child type, or the parent already has
    the maximum number of children allowed. E.g., containers can only have one child, splitters can
    only have two, and sizers can have an unlimited number.

*/

// The parent parameter is used to determine if the parent allows this type of child, and if so how
// many of those children are allowed. The second part of the pair is a Node:: error code (see enum
// in node.h).
auto NodeCreator::CreateNode(GenName name, Node* parent, bool verify_language_support)
    -> std::pair<NodeSharedPtr, int>
{
    ASSERT(name != gen_unknown);
    if (name == gen_unknown)
    {
        return { NodeSharedPtr(), 0 };
    }

    auto* node_decl = ResolveNodeDeclaration(name);
    if (!node_decl)
    {
        return { NodeSharedPtr(), Node::unknown_gen_name };
    }

    if (!parent)
    {
        return { NewNode(node_decl), 0 };
    }

    // This happens when importing wxFormBuilder projects
    if (is_OldHostType(node_decl->get_DeclName()))
    {
        return { NewNode(node_decl), 0 };
    }

    auto parent_error = ValidateParentConstraints(name, node_decl, parent);
    if (parent_error != 0)
    {
        return { NodeSharedPtr(), parent_error };
    }

    auto node = AllocateChildNode(name, node_decl, parent);

    if (verify_language_support && node)
    {
        auto language_error = VerifyLanguageSupport(node);
        if (language_error != 0)
        {
            return { NodeSharedPtr(), language_error };
        }
    }

    return { node, 0 };
}

auto NodeCreator::ResolveNodeDeclaration(GenName name) const -> NodeDeclaration*
{
    // This is a way for a ribbon panel button to indicate a wxBoxSizer with vertical orientation
    if (name == gen_VerticalBoxSizer)
    {
        return m_a_declarations[gen_wxBoxSizer];
    }

    auto* node_decl = m_a_declarations.at(name);
    if (node_decl)
    {
        return node_decl;
    }

    // Unless the toolbar is a child of a wxAui frame window, there's little to no difference
    // between a wxAuiToolBar and a wxToolBar. Checking it here allows us to automatically
    // convert imported projects, and then if we ever do decide to support wxAuiToolBar, imports
    // will immediately switch without having to touch the import code.

    if (name == gen_wxAuiToolBar)
    {
        return m_a_declarations[gen_wxToolBar];
    }

    return nullptr;
}

auto NodeCreator::ValidateParentConstraints(GenName name, NodeDeclaration* node_decl, Node* parent)
    -> int
{
    // Check for widgets which can ONLY have a frame for a parent.
    if (node_decl->is_Type(type_statusbar) || node_decl->is_Type(type_menubar) ||
        node_decl->is_Type(type_toolbar))
    {
        if (parent->is_Type(type_form) && !parent->is_Type(type_frame_form))
        {
            return Node::parent_not_wxFrame;
        }
    }
    else if (parent->is_Type(type_tool))
    {
        auto* grand_parent = parent->get_Parent();
        if (grand_parent->is_Gen(gen_wxToolBar) && node_decl->is_Type(type_menu))
        {
            return Node::invalid_tool_grandparent;
        }
    }
    else if (name == gen_BookPage && parent->is_Type(type_bookpage))
    {
        auto* grand_parent = parent->get_Parent();
        if (!grand_parent || !grand_parent->is_Gen(gen_wxTreebook))
        {
            return Node::invalid_page_grandparent;
        }
    }

    return 0;
}

auto NodeCreator::AllocateChildNode(GenName name, NodeDeclaration* node_decl, Node* parent)
    -> NodeSharedPtr
{
    auto max_children = parent->get_AllowableChildren(node_decl->get_GenType());

    if (max_children == child_count::infinite)
    {
        auto node = NewNode(node_decl);
        if (name == gen_VerticalBoxSizer)
        {
            node->set_value(prop_orientation, "wxVERTICAL");
        }
        return node;
    }

    if (max_children == child_count::none)
    {
        // The parent doesn't allow this child type. Return empty NodeSharedPtr.
        // It's the caller's responsibility to try to find a proper parent.
        return {};
    }

    if (node_decl->is_Type(type_sizer) || node_decl->is_Type(type_gbsizer))
    {
        auto count = CountChildrenWithSameType(parent, node_decl->get_GenType());
        if (count < (to_size_t) max_children)
        {
            auto node = NewNode(node_decl);
            if (name == gen_VerticalBoxSizer)
            {
                node->set_value(prop_orientation, "wxVERTICAL");
            }
            return node;
        }
    }
    else if (parent->is_Gen(gen_wxSplitterWindow))
    {
        // for splitters, we only care if the type is allowed, and if the splitter only has one
        // child so far.
        if (parent->get_ChildCount() < 2)
        {
            return NewNode(node_decl);
        }
    }
    else
    {
        auto count = CountChildrenWithSameType(parent, node_decl->get_GenType());
        if (count < (to_size_t) max_children)
        {
            return NewNode(node_decl);
        }
    }

    return {};
}

auto NodeCreator::VerifyLanguageSupport(NodeSharedPtr& node) -> int
{
    auto* gen = node->get_Generator();
    if (!gen)
    {
        return 0;
    }

    auto result = gen->isLanguageVersionSupported(Project.get_CodePreference());
    if (result.first)
    {
        return 0;
    }

    if (wxMessageBox(result.second + ". Create anyway?", "Unsupported widget",
                     wxYES_NO | wxICON_QUESTION) == wxNO)
    {
        // Because node only has a single reference, it will be deleted when it goes out
        // of scope via this return.
        return Node::unsupported_language;
    }

    return 0;
}

auto NodeCreator::is_ValidCreateParent(GenName name, Node* parent, bool use_recursion) const
    -> Node*
{
    ASSERT(name != gen_unknown);
    if (name == gen_unknown)
    {
        return nullptr;
    }

    ASSERT(parent);
    if (!parent)
    {
        return nullptr;
    }

    auto* node_decl = ResolveNodeDeclaration(name);
    if (!node_decl)
    {
        return nullptr;
    }

    // Check for widgets which can ONLY have a frame for a parent.
    if (node_decl->is_Type(type_statusbar) || node_decl->is_Type(type_menubar) ||
        node_decl->is_Type(type_toolbar))
    {
        if (!parent->is_Type(type_frame_form))
        {
            return parent->get_Parent();
        }
    }
    else if (parent->is_Type(type_tool))
    {
        auto* grand_parent = parent->get_Parent();
        if (grand_parent->is_Gen(gen_wxToolBar) && node_decl->is_Type(type_menu))
        {
            return nullptr;
        }
    }
    else if (name == gen_BookPage && parent->is_Type(type_bookpage))
    {
        if (auto* grandfather = parent->get_Parent(); grandfather)
        {
            return is_ValidCreateParent(name, grandfather);
        }

        return nullptr;
    }

    if (CanParentAcceptChild(node_decl, parent))
    {
        return parent;
    }

    if (use_recursion)
    {
        if (auto* grandfather = parent->get_Parent(); grandfather)
        {
            return is_ValidCreateParent(name, grandfather);
        }
    }

    return nullptr;
}

auto NodeCreator::CanParentAcceptChild(NodeDeclaration* node_decl, Node* parent) -> bool
{
    auto max_children = parent->get_AllowableChildren(node_decl->get_GenType());

    if (max_children == child_count::infinite)
    {
        return true;
    }

    if (max_children == child_count::none)
    {
        return false;
    }

    if (node_decl->is_Type(type_sizer) || node_decl->is_Type(type_gbsizer))
    {
        auto count = CountChildrenWithSameType(parent, node_decl->get_GenType());
        return count < (to_size_t) max_children;
    }

    if (parent->is_Gen(gen_wxSplitterWindow))
    {
        // for splitters, we only care if the type is allowed, and if the splitter only has one
        // child so far.
        return parent->get_ChildCount() < 2;
    }

    auto count = CountChildrenWithSameType(parent, node_decl->get_GenType());
    return count < (to_size_t) max_children;
}

// Called when the GenName isn't availalble
auto NodeCreator::CreateNode(std::string_view name, Node* parent, bool verify_language_support)
    -> std::pair<NodeSharedPtr, int>
{
    if (auto result = rmap_GenNames.find(name); result != rmap_GenNames.end())
    {
        return CreateNode(result->second, parent, verify_language_support);
    }

    FAIL_MSG(std::format("No component definition for {}", name));
    return { NodeSharedPtr(), Node::unknown_gen_name };  // -1 indicates failure
}

auto NodeCreator::MakeCopy(Node* node, Node* parent) -> NodeSharedPtr
{
    ASSERT(node);

    auto copyObj = CreateToolCopy(node, parent);
    if (!copyObj)
    {
        copyObj = NewNode(node->get_NodeDeclaration());
    }

    ASSERT(copyObj);

    CopyProperties(node, copyObj);
    copyObj->CopyEventsFrom(node);

    // It will be rare, but sometimes a user may want to copy a form such as FormPanel and paste it
    // into a sizer or a book. In that case, we need to create the non-form version of the control.
    if (parent && !parent->is_Gen(gen_Project) && node->is_Form())
    {
        if (auto child_object = ConvertFormToControl(node, parent); child_object)
        {
            CopyProperties(node, child_object);
            child_object->CopyEventsFrom(node);
            copyObj = child_object;
        }
    }

    CopyChildren(node, copyObj);

    return copyObj;
}

auto NodeCreator::CreateToolCopy(Node* node, Node* parent) -> NodeSharedPtr
{
    if (!parent)
    {
        return nullptr;
    }

    // Sometimes we need to copy a similar node to a new node using the parent as the guide.
    if (node->is_Gen(gen_tool) &&
        (parent->is_Gen(gen_wxAuiToolBar) || parent->is_Gen(gen_AuiToolBar)))
    {
        return NewNode(gen_auitool);
    }

    if (node->is_Gen(gen_auitool) && (parent->is_Gen(gen_wxToolBar) || parent->is_Gen(gen_ToolBar)))
    {
        return NewNode(gen_tool);
    }

    return nullptr;
}

void NodeCreator::CopyProperties(Node* source, NodeSharedPtr& target)
{
    for (auto& iter: source->get_PropsVector())
    {
        if (auto* copyProp = target->get_PropPtr(iter.get_name()); copyProp)
        {
            copyProp->set_value(iter.as_view());
        }
    }
}

auto NodeCreator::ConvertFormToControl(Node* node, Node* parent) -> NodeSharedPtr
{
    NodeSharedPtr child_object;

    if (node->is_Gen(gen_ToolBar))
    {
        child_object = CreateNode(gen_wxToolBar, parent).first;
        // REVIEW: [Randalphwa - 10-06-2022] This will fail if the parent is a wxFrame and it
        // already has a toolbar. Should we let the user know?
    }
    else if (node->is_Gen(gen_MenuBar))
    {
        child_object = CreateNode(gen_wxMenuBar, parent).first;
    }
    else if (node->is_Gen(gen_RibbonBar))
    {
        child_object = CreateNode(gen_wxRibbonBar, parent).first;
    }
    else if (node->is_Gen(gen_PanelForm))
    {
        if (parent->is_Type(type_choicebook) || parent->is_Type(type_listbook) ||
            parent->is_Type(type_notebook) || parent->is_Type(type_simplebook))
        {
            child_object = CreateNode(gen_BookPage, parent).first;
        }
        else
        {
            child_object = CreateNode(gen_wxPanel, parent).first;
        }
    }

    return child_object;
}

void NodeCreator::CopyChildren(Node* source, NodeSharedPtr& target)
{
    for (auto& child: source->get_ChildNodePtrs())
    {
        if (auto childCopy = MakeCopy(child.get()); childCopy)
        {
            target->AdoptChild(childCopy);
        }
    }
}

// auto NodeCreator::get_ConstantAsInt(const std::string& name, int defValue) const -> int
auto NodeCreator::get_ConstantAsInt(std::string_view name, int defValue) const -> int
{
    if (auto iter = m_map_constants.find(name); iter != m_map_constants.end())
    {
        return iter->second;
    }
    return defValue;
}
