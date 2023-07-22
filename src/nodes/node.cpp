/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains user-modifiable node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/sizer.h>    // provide wxSizer class for layout

#include "node.h"

#include "gridbag_item.h"     // GridBagItem -- Dialog for inserting an item into a wxGridBagSizer node
#include "image_handler.h"    // ProjectImage class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node_creator.h"     // NodeCreator class
#include "node_decl.h"        // NodeDeclaration class
#include "node_gridbag.h"     // GridBag -- Create and modify a node containing a wxGridBagSizer
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // InsertNodeAction -- Undoable command classes derived from UndoAction

using namespace GenEnum;

// clang-format off
inline const GenType lst_form_types[] =
{

    type_DocViewApp,
    type_wx_document,
    type_wx_view,

    type_aui_toolbar_form,
    type_form,
    type_frame_form,
    type_images,
    type_menubar_form,
    type_popup_menu,
    type_ribbonbar_form,
    type_toolbar_form,
    type_wizard,

};

const std::vector<std::string> reserved_names = {
    "bitmaps",  // used for wxBitmapBundle
    "_svg_string_", // used for python SVG image processing

    "bundle_list",  // used for wxBitmapBundle, primarily for books

    // These are used when generating Python code for wxBitmapBundle
    "bundle_1",
    "bundle_2",
    "bundle_3",
    "bundle_4",
    "bundle_5",
    "bundle_6",
    "bundle_7",
    "bundle_8",
    "bundle_9",

    "idx",      // used for wxListItem
    "info"      // used for wxListItem
};

// clang-format on

bool Node::isForm() const noexcept
{
    for (auto& iter: lst_form_types)
    {
        if (isType(iter))
            return true;
    }
    return false;
}

Node::Node(NodeDeclaration* declaration) : m_declaration(declaration) {}

NodeProperty* Node::getPropPtr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return &m_properties[result->second];
    else
        return nullptr;
}

NodeEvent* Node::getEvent(tt_string_view name)
{
    if (auto iter = m_map_events.find(name); iter != m_map_events.end())
    {
        return &iter->second;
    }
    else
    {
        return nullptr;
    }
}

size_t Node::getInUseEventCount() const
{
    size_t count = 0;

    for (auto& iter: m_map_events)
    {
        if (iter.second.get_value().size())
            ++count;
    }

    return count;
}

NodeProperty* Node::addNodeProperty(PropDeclaration* declaration)
{
    auto& prop = m_properties.emplace_back(declaration, this);
    m_prop_indices[prop.get_name()] = (m_properties.size() - 1);
    return &m_properties[m_properties.size() - 1];
}

void Node::addNodeEvent(const NodeEventInfo* info)
{
    m_map_events.emplace(info->get_name(), NodeEvent(info, this));
}

void Node::copyEventsFrom(Node* from)
{
    ASSERT(from);
    for (auto& iter: from->m_map_events)
    {
        if (iter.second.get_value().size())
        {
            auto event = getEvent(iter.second.get_name());
            event->set_value(iter.second.get_value());
        }
    }
}

Node* Node::getForm() noexcept
{
    if (isForm())
    {
        return this;
    }

    auto parent = getParent();
    while (parent)
    {
        if (parent->isForm())
        {
            return parent;
        }
        parent = parent->getParent();
    }

    return nullptr;
}

Node* Node::getFolder() noexcept
{
    auto parent = getParent();
    while (parent)
    {
        if (parent->isGen(gen_folder))
        {
            return parent;
        }
        parent = parent->getParent();
    }

    return nullptr;
}

Node* Node::getValidFormParent() noexcept
{
    auto parent = this;
    while (parent)
    {
        if (parent->isFormParent())
        {
            return parent;
        }
        parent = parent->getParent();
    }

    return nullptr;
}

bool Node::adoptChild(NodeSharedPtr child)
{
    ASSERT_MSG(child != getSharedPtr(), "A node can't adopt itself!");
    if (isChildAllowed(child))
    {
        m_children.push_back(child);
        child->setParent(getSharedPtr());
        return true;
    }

    return false;
}

bool Node::addChild(NodeSharedPtr node)
{
    if (isChildAllowed(node))
    {
        m_children.push_back(node);
        return true;
    }

    return false;
}

bool Node::addChild(Node* node)
{
    if (isChildAllowed(node))
    {
        m_children.push_back(node->getSharedPtr());
        return true;
    }

    return false;
}

bool Node::addChild(size_t idx, NodeSharedPtr node)
{
    if (isChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node);
        return true;
    }

    return false;
}

bool Node::addChild(size_t idx, Node* node)
{
    if (isChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node->getSharedPtr());
        return true;
    }

    return false;
}

bool Node::isChildAllowed(NodeDeclaration* child)
{
    ASSERT(child);

    auto max_children = m_declaration->getAllowableChildren(child->getGenType());

    if (max_children == child_count::none)
        return false;

    if (max_children == child_count::infinite)
        return true;

    if (isGen(gen_wxSplitterWindow))
        return (getChildCount() < 2);

    // Because m_children contains shared_ptrs, we don't want to use an iteration loop which will get/release the shared
    // ptr. Using an index into the vector lets us access the raw pointer.

    ptrdiff_t children = 0;
    for (size_t i = 0; i < m_children.size() && children <= max_children; ++i)
    {
        if (getChild(i)->getGenType() == child->getGenType())
            ++children;

        // treat type-sizer and type_gbsizer as the same since forms and contains can only have one of them as the top level
        // sizer.

        else if (child->getGenType() == type_sizer && getChild(i)->getGenType() == type_gbsizer)
            ++children;
        else if (child->getGenType() == type_gbsizer && getChild(i)->getGenType() == type_sizer)
            ++children;
    }

    if (children >= max_children)
        return false;

    return true;
}

bool Node::isChildAllowed(Node* child)
{
    ASSERT(child);

    return isChildAllowed(child->getNodeDeclaration());
}

void Node::removeChild(Node* node)
{
    for (size_t pos = 0; const auto& child: m_children)
    {
        if (child.get() == node)
        {
            m_children.erase(m_children.begin() + pos);
            break;
        }
        ++pos;
        ASSERT_MSG(pos < m_children.size(), "failed to find child node!");
    }
}

void Node::removeChild(size_t pos)
{
    ASSERT(pos < m_children.size());

    auto iter = m_children.begin() + pos;
    m_children.erase(iter);
}

size_t Node::getChildPosition(Node* node)
{
    for (size_t pos = 0; const auto& child: m_children)
    {
        if (child.get() == node)
        {
            return pos;
        }
        ++pos;
    }

    // REVIEW: [Randalphwa - 06-13-2022] Actually, this is sometimes valid when using undo. What really should happen is
    // that it should return int64_t so that -1 becomes a valid return.

    // FAIL_MSG("failed to find child node, returned position is invalid!")
    return (m_children.size() - 1);
}

bool Node::changeChildPosition(NodeSharedPtr node, size_t pos)
{
    size_t cur_pos = getChildPosition(node.get());

    if (cur_pos == getChildCount() || pos >= getChildCount())
        return false;

    if (pos == cur_pos)
        return true;

    removeChild(node);
    addChild(pos, node);
    return true;
}

bool Node::isLocal() const noexcept
{
    return isPropValue(prop_class_access, "none");
}

bool Node::hasValue(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].hasValue();
    else
        return false;
}

bool Node::isPropValue(PropName name, const char* value) const noexcept
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_string().is_sameas(value));
    }

    return false;
}

bool Node::isPropValue(PropName name, bool value) const noexcept
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_bool() == value);
    }

    return false;
}

bool Node::isPropValue(PropName name, int value) const noexcept
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_int() == value);
    }

    return false;
}

int Node::as_mockup(PropName name, std::string_view prefix) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_mockup(prefix);
    else
        return 0;
}

std::vector<tt_string> Node::as_ArrayString(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_ArrayString();
    else
        return std::vector<tt_string>();
}

tt_string* Node::getPropValuePtr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_raw_ptr();
    else
        return nullptr;
}

tt_string Node::getPropId() const
{
    tt_string id;
    if (auto result = m_prop_indices.find(prop_id); result != m_prop_indices.end())
        id = m_properties[result->second].getPropId();
    return id;
}

std::vector<NODEPROP_STATUSBAR_FIELD> Node::as_statusbar_fields(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_statusbar_fields();
    else
        return std::vector<NODEPROP_STATUSBAR_FIELD>();
}

std::vector<NODEPROP_CHECKLIST_ITEM> Node::as_checklist_items(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_checklist_items();
    else
        return std::vector<NODEPROP_CHECKLIST_ITEM>();
}

std::vector<NODEPROP_RADIOBOX_ITEM> Node::as_radiobox_items(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_radiobox_items();
    else
        return std::vector<NODEPROP_RADIOBOX_ITEM>();
}

std::vector<NODEPROP_BMP_COMBO_ITEM> Node::as_bmp_combo_items(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_bmp_combo_items();
    else
        return std::vector<NODEPROP_BMP_COMBO_ITEM>();
}

const tt_string& Node::getPropDefaultValue(PropName name)
{
    auto prop = getPropPtr(name);

    ASSERT_MSG(prop, tt_string(getNodeName()) << " doesn't have the property " << map_PropNames[name]);

    if (prop)
        return prop->GetDefaultValue();
    else
        return tt_empty_cstr;
}

const tt_string& Node::getNodeName() const
{
    if (auto it = m_prop_indices.find(prop_var_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else if (it = m_prop_indices.find(prop_class_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else
        return tt_empty_cstr;
}

const tt_string& Node::getParentName() const
{
    if (m_parent)
        return m_parent->getNodeName();

    return tt_empty_cstr;
}

const tt_string& Node::getFormName()
{
    if (auto form = getForm(); form)
    {
        return form->getNodeName();
    }
    return tt_empty_cstr;
}

wxSizerFlags Node::getSizerFlags() const
{
    wxSizerFlags flags;
    flags.Proportion(as_int(prop_proportion));
    auto border_size = as_int(prop_border_size);
    int direction = 0;
    auto& border_settings = as_string(prop_borders);
    if (border_settings.contains("wxALL"))
    {
        direction = wxALL;
    }
    else
    {
        if (border_settings.contains("wxLEFT"))
            direction |= wxLEFT;
        if (border_settings.contains("wxRIGHT"))
            direction |= wxRIGHT;
        if (border_settings.contains("wxTOP"))
            direction |= wxTOP;
        if (border_settings.contains("wxBOTTOM"))
            direction |= wxBOTTOM;
    }
    flags.Border(direction, border_size);

    if (auto& alignment = as_string(prop_alignment); alignment.size())
    {
        if (alignment.contains("wxALIGN_CENTER"))
        {
            if (alignment.contains("wxALIGN_CENTER_VERTICAL") && alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
                flags.Center();
            else if (alignment.contains("wxALIGN_CENTER_VERTICAL"))
                flags.CenterVertical();
            else if (alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
                flags.CenterHorizontal();
            else
                flags.Center();
        }
        else
        {
            if (alignment.contains("wxALIGN_LEFT"))
                flags.Left();
            if (alignment.contains("wxALIGN_RIGHT"))
                flags.Right();
            if (alignment.contains("wxALIGN_TOP"))
                flags.Top();
            if (alignment.contains("wxALIGN_BOTTOM"))
                flags.Bottom();
        }
    }

    if (auto& prop = as_string(prop_flags); prop.size())
    {
        if (prop.contains("wxEXPAND"))
            flags.Expand();
        if (prop.contains("wxSHAPED"))
            flags.Shaped();
        if (prop.contains("wxFIXED_MINSIZE"))
            flags.FixedMinSize();
        if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
            flags.ReserveSpaceEvenIfHidden();
    }

    return flags;
}

Node* Node::createChildNode(GenName name)
{
    auto& frame = wxGetFrame();

    auto new_node = NodeCreation.createNode(name, this);

    Node* parent = this;

    if (!new_node)
    {
        if ((isForm() || isContainer()) && getChildCount())
        {
            if (getChild(0)->getGenType() == type_sizer || getChild(0)->getGenType() == type_gbsizer)
            {
                new_node = NodeCreation.createNode(name, getChild(0));
                if (!new_node)
                    return nullptr;
                parent = getChild(0);
            }

            if (parent->getGenType() == type_gbsizer)
            {
                GridBag grid_bag(parent);
                if (grid_bag.InsertNode(parent, new_node.get()))
                    return new_node.get();
                else
                {
                    return nullptr;
                }
            }
        }
    }

    if (new_node)
    {
        if (isGen(gen_wxGridBagSizer))
        {
            GridBag grid_bag(this);
            if (grid_bag.InsertNode(this, new_node.get()))
                return new_node.get();
            else
            {
                return nullptr;
            }
        }

#if defined(_WIN32)

        // In a Windows build, the default background colour of white doesn't match the normal background color of the parent
        // so we set it to the more normal Windows colour.

        // REVIEW: [KeyWorks - 03-17-2021] Need to figure out a better way to do this which is cross platform. As it
        // currently exists, if the Windows version of wxUiEditor is used, then ALL versions of the app the user creates will
        // use this background color.

        if (name == gen_BookPage)
        {
            if (auto prop = new_node->getPropPtr(prop_background_colour); prop)
            {
                prop->set_value("wxSYS_COLOUR_BTNFACE");

                // REVIEW: [Randalphwa - 06-18-2023] The node has not been inserted yet, so
                // firing a prop change event can result in a hang, particularly when
                // createChildNode() was called while creating a new Book container.

                // frame.FirePropChangeEvent(prop);
            }
        }
#endif  // _WIN32

        tt_string undo_str;
        undo_str << "insert " << map_GenNames[name];
        frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str));
    }

    // A "ribbonButton" component is used for both wxRibbonButtonBar and wxRibbonToolBar. If creating the node failed,
    // then assume the parent is wxRibbonToolBar and retry with "ribbonTool"
    else if (name == gen_ribbonButton)
    {
        new_node = NodeCreation.createNode(gen_ribbonTool, this);
        if (new_node)
        {
            tt_string undo_str = "insert ribbon tool";
            frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), this, undo_str));
        }
        else
        {
            return nullptr;
        }
    }
    else
    {
        // If we get here, then the current node cannot be a parent of the new node. We look at the current node's
        // parent and see if it accepts the new node as a child. This deals with the case where the user selected a
        // widget, and wants a new widget created right after the selected widget with both having the same parent
        // (typically a sizer).

        parent = getParent();

        if (parent)
        {
            auto decl = NodeCreation.get_declaration(name);
            auto max_children = getNodeDeclaration()->getAllowableChildren(decl->getGenType());
            auto cur_children = NodeCreation.CountChildrenWithSameType(this, decl->getGenType());
            if (max_children > 0 && cur_children >= static_cast<size_t>(max_children))
            {
                if (isGen(gen_wxSplitterWindow))
                {
                    wxMessageBox("You cannot add more than two windows to a splitter window.", "Cannot add control");
                }
                else
                {
                    wxMessageBox(tt_string() << "You can only add " << (to_size_t) max_children << ' ' << map_GenNames[name]
                                             << " as a child of " << declName());
                }

                return nullptr;
            }

            new_node = NodeCreation.createNode(name, parent);
            if (new_node)
            {
                if (parent->isGen(gen_wxGridBagSizer))
                {
                    GridBag grid_bag(parent);
                    if (grid_bag.InsertNode(parent, new_node.get()))
                        return new_node.get();
                    else
                    {
                        return nullptr;
                    }
                }

                auto pos = parent->findInsertionPos(this);
                tt_string undo_str;
                undo_str << "insert " << map_GenNames[name];
                frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
            }
        }
        else
        {
            wxMessageBox(tt_string() << "You cannot add " << map_GenNames[name] << " as a child of " << declName());
            return nullptr;
        }
    }

    if (new_node)
    {
        if (Project.get_PreferredLanguage() != GEN_LANG_CPLUSPLUS)
        {
            tt_string member_name = new_node->as_string(prop_var_name);
            if (member_name.starts_with("m_"))
            {
                member_name.erase(0, 2);
                if (member_name.ends_with("_2"))
                {
                    // This is unlikely, but the previous check for duplication assumed a m_
                    // prefix, so without the prefix, it's possible that the name isn't a
                    // duplicate. We only check for _2 since a mix of names with/without a m_
                    // prefix is unlikely.
                    member_name.erase(member_name.size() - 2);
                }
                new_node->set_value(prop_var_name, member_name);
                new_node->fixDuplicateName();
            }
        }

        frame.FireCreatedEvent(new_node.get());
        frame.SelectNode(new_node.get(), evt_flags::fire_event | evt_flags::force_selection);
    }
    return new_node.get();
}

Node* Node::createNode(GenName name)
{
    auto& frame = wxGetFrame();
    auto cur_selection = frame.GetSelectedNode();
    if (!cur_selection)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return nullptr;
    }
    return cur_selection->createChildNode(name);
}

void Node::modifyProperty(PropName name, tt_string_view value)
{
    auto prop = getPropPtr(name);
    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::modifyProperty(tt_string_view name, int value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = getPropPtr(find_prop->second);

    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::modifyProperty(tt_string_view name, tt_string_view value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = getPropPtr(find_prop->second);

    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::modifyProperty(NodeProperty* prop, int value)
{
    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::modifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

tt_string Node::getUniqueName(const tt_string& proposed_name, PropName prop_name)
{
    tt_string new_name(proposed_name);
    if (isForm())
        return {};

    auto form = getForm();
    if (!form)
        return {};

    std::unordered_set<std::string> name_set;

    if (prop_name == prop_var_name)
    {
        for (auto& iter: reserved_names)
        {
            name_set.emplace(iter);
        }

        form->collectUniqueNames(name_set, this);
    }
    else if (isGen(gen_propGridItem) || isGen(gen_propGridCategory))
    {
        auto parent = getParent();
        if (parent->isGen(gen_propGridPage))
        {
            parent = parent->getParent();
        }

        parent->collectUniqueNames(name_set, this, prop_name);
    }
    else
    {
        FAIL_MSG("unsupported prop_name");
        return new_name;
    }

    if (auto it = name_set.find(new_name); it != name_set.end())
    {
        // We get here if the name has already been used.

        std::string org_name(proposed_name);
        while (tt::is_digit(org_name.back()))
        {
            // remove any trailing digits
            org_name.erase(org_name.size() - 1, 1);
        }

        for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
        {
            new_name.clear();
            if (org_name.back() == '_')
            {
                org_name.pop_back();
            }
            new_name << org_name << '_' << i;
        }
    }

    // We return this name whether or not it has actually changed.
    return new_name;
}

static const PropName s_var_names[] = {

    prop_var_name,
    prop_checkbox_var_name,
    prop_radiobtn_var_name,
    prop_validator_variable,

};

bool Node::fixDuplicateName()
{
    if (isType(type_form) || isType(type_frame_form) || isType(type_menubar_form) || isType(type_ribbonbar_form) ||
        isType(type_toolbar_form) || isType(type_aui_toolbar_form) || isType(type_wizard) || isType(type_popup_menu) ||
        isType(type_project))
    {
        return false;
    }

    auto form = getForm();
    ASSERT(form);
    if (!form)
        return false;

    std::unordered_set<std::string> name_set;
    for (auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }

    form->collectUniqueNames(name_set, this);

    bool replaced = false;
    for (auto& iter: s_var_names)
    {
        if (auto& name = as_string(iter); name.size())
        {
            if (auto it = name_set.find(name); it != name_set.end())
            {
                // We get here if the name has already been used.

                std::string org_name(name);
                while (tt::is_digit(org_name.back()))
                {
                    // remove any trailing digits
                    org_name.erase(org_name.size() - 1, 1);
                }
                if (org_name.back() == '_')
                    org_name.erase(org_name.size() - 1, 1);

                tt_string new_name;
                for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
                {
                    new_name.clear();
                    new_name << org_name << '_' << i;
                }

                auto fix_name = getPropPtr(iter);
                fix_name->set_value(new_name);
                replaced = true;
            }
        }
    }

    if (isGen(gen_propGridItem) || isGen(gen_propGridCategory))
    {
        name_set.clear();
        form->collectUniqueNames(name_set, this, prop_label);

        tt_string org_name(as_string(prop_label));
        auto result = getUniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto fix_name = getPropPtr(prop_label);
            fix_name->set_value(result);
        }
    }

    return replaced;
}

void Node::fixDuplicateNodeNames(Node* form)
{
    if (!form)
    {
        if (isForm())
        {
            for (auto& child: getChildNodePtrs())
            {
                child->fixDuplicateNodeNames(this);
            }
            return;
        }
        else
        {
            form = getForm();
        }

        ASSERT(form);
        if (!form)
            return;
    }

    // Collect all variable names except for those in the current node. Then check if any of the variable names in the
    // current node are duplicated in other nodes, and if so fix them. Then step into each of the child nodes recursively
    // until all nodes have been processed. The reason for collecting the names for each set is because there could be
    // multiple identical names if the node was generated by one of the CreateNew functions. Even pasting in from wxSmith or
    // wxFormBuilder could have multiple identical names if they didn't do their own name fixups correctly.

    std::unordered_set<std::string> name_set;
    for (auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }

    form->collectUniqueNames(name_set, this);

    for (auto& iter: s_var_names)
    {
        if (auto& name = as_string(iter); name.size())
        {
            if (auto it = name_set.find(name); it != name_set.end())
            {
                // We get here if the name has already been used.

                std::string org_name(name);
                while (tt::is_digit(org_name.back()))
                {
                    // remove any trailing digits
                    org_name.erase(org_name.size() - 1, 1);
                }
                if (org_name.back() == '_')
                    org_name.erase(org_name.size() - 1, 1);

                tt_string new_name;
                for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
                {
                    new_name.clear();
                    new_name << org_name << '_' << i;
                }

                auto fix_name = getPropPtr(iter);
                fix_name->set_value(new_name);
            }
        }
    }

    for (const auto& child: getChildNodePtrs())
    {
        child->fixDuplicateNodeNames(form);
    }

    if (isGen(gen_propGridItem) || isGen(gen_propGridCategory))
    {
        name_set.clear();
        auto parent = getParent();
        if (parent->isGen(gen_propGridPage))
        {
            parent = parent->getParent();
        }

        parent->collectUniqueNames(name_set, this, prop_label);

        tt_string org_name(as_string(prop_label));
        auto result = getUniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto fix_name = getPropPtr(prop_label);
            fix_name->set_value(result);
        }
    }
}

void Node::collectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node, PropName prop_name)
{
    if (!isForm() && cur_node != this && !isGen(gen_wxPropertyGrid) && !isGen(gen_wxPropertyGridManager))
    {
        if (prop_name == prop_var_name)
        {
            for (auto& iter: s_var_names)
            {
                if (auto& name = as_string(iter); name.size())
                {
                    name_set.emplace(name);
                }
            }
        }
        else
        {
            if (auto& name = as_string(prop_name); name.size())
            {
                name_set.emplace(name);
            }
        }
    }

    for (const auto& iter: getChildNodePtrs())
    {
        iter->collectUniqueNames(name_set, cur_node, prop_name);
    }
}

ptrdiff_t Node::findInsertionPos(Node* child) const
{
    if (child)
    {
        for (size_t new_pos = 0; new_pos < getChildCount(); ++new_pos)
        {
            if (getChild(new_pos) == child)
                return new_pos + 1;
        }
    }
    return -1;
}

size_t Node::getNodeSize() const
{
    auto size = sizeof(*this);
    // Add the size of all the node pointers, but not the size of the individual children
    size += (m_children.size() * sizeof(void*));

    for (auto& iter: m_properties)
    {
        size += iter.GetPropSize();
    }

    for (auto& iter: m_map_events)
    {
        size += iter.second.GetEventSize();
    }

    size += (m_prop_indices.size() * (sizeof(size_t) * 2));

    return size;
}

// Create a hash of the node name and all property values of the node, and recursively call all children
void Node::calcNodeHash(size_t& hash) const
{
    // djb2 hash algorithm

    if (hash == 0)
        hash = 5381;

    for (auto iter: getNodeName())
        hash = ((hash << 5) + hash) ^ iter;

    for (auto prop: m_properties)
    {
        for (auto char_iter: prop.as_string())
            hash = ((hash << 5) + hash) ^ char_iter;
    }

    for (auto child: m_children)
    {
        child->calcNodeHash(hash);
    }
}

std::vector<NodeProperty*> Node::findAllChildProperties(PropName name)
{
    std::vector<NodeProperty*> result;

    findAllChildProperties(result, name);

    return result;
}

void Node::findAllChildProperties(std::vector<NodeProperty*>& list, PropName name)
{
    for (const auto& child: m_children)
    {
        if (child->hasValue(name))
        {
            list.emplace_back(child->getPropPtr(name));
        }
        if (child->getChildCount())
        {
            child->findAllChildProperties(list, name);
        }
    }
}
