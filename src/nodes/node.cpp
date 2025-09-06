/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains user-modifiable node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/sizer.h>    // provide wxSizer class for layout

#include "node.h"

#include "gridbag_item.h"     // Dialog for inserting an item into a wxGridBagSizer node
#include "mainframe.h"        // Main window frame
#include "node_creator.h"     // NodeCreator class
#include "node_decl.h"        // NodeDeclaration class
#include "node_gridbag.h"     // Create and modify a node containing a wxGridBagSizer
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "preferences.h"      // Preferences -- Stores user preferences
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Miscellaneous utilities

using namespace GenEnum;

// clang-format off
inline const GenType lst_form_types[] =
{

    type_DocViewApp,
    type_wx_document,
    type_wx_view,

    type_aui_toolbar_form,
    type_data_list,
    type_form,
    type_frame_form,
    type_images,
    type_menubar_form,
    type_panel_form,
    type_popup_menu,
    type_propsheetform,
    type_ribbonbar_form,
    type_toolbar_form,
    type_wizard,

};

const std::vector<std::string> reserved_names = {
    "bitmaps",       // used for wxBitmapBundle
    "_svg_string_",  // used for python SVG image processing

    // Python variables
    "_OK",
    "_Yes",
    "_Save",
    "_Cancel",
    "_No",
    "_Close",
    "_Help",
    "_ContextHelp",

    // Ruby variables
    "_ok_btn",
    "_yes_btn",
    "_save_btn",
    "_cancel_btn",
    "_no_btn",
    "_close_btn",
    "_help_btn",
    "_context_help_btn",

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

bool Node::is_Form() const noexcept
{
    for (auto& iter: lst_form_types)
    {
        if (is_Type(iter))
            return true;
    }
    return false;
}

Node::Node(NodeDeclaration* declaration) : m_declaration(declaration) {}

NodeProperty* Node::get_PropPtr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return &m_properties[result->second];
    else
        return nullptr;
}

NodeEvent* Node::get_Event(tt_string_view name)
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

size_t Node::get_InUseEventCount() const
{
    size_t count = 0;

    for (auto& iter: m_map_events)
    {
        if (iter.second.get_value().size())
            ++count;
    }

    return count;
}

NodeProperty* Node::AddNodeProperty(PropDeclaration* declaration)
{
    auto& prop = m_properties.emplace_back(declaration, this);
    m_prop_indices[prop.get_name()] = (m_properties.size() - 1);
    return &m_properties[m_properties.size() - 1];
}

void Node::AddNodeEvent(const NodeEventInfo* info)
{
    m_map_events.emplace(info->get_name(), NodeEvent(info, this));
}

void Node::CopyEventsFrom(Node* from)
{
    ASSERT(from);
    for (auto& iter: from->m_map_events)
    {
        if (iter.second.get_value().size())
        {
            if (auto* event = get_Event(iter.second.get_name()); event)
            {
                event->set_value(iter.second.get_value());
            }
        }
    }
}

Node* Node::get_Form() noexcept
{
    if (is_Form())
    {
        return this;
    }

    for (auto parent = get_Parent(); parent; parent = parent->get_Parent())
    {
        if (parent->is_Form())
        {
            return parent;
        }
    }

    return nullptr;
}

Node* Node::get_Folder() noexcept
{
    for (auto parent = get_Parent(); parent; parent = parent->get_Parent())
    {
        if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_data_folder))
        {
            return parent;
        }
    }

    return nullptr;
}

Node* Node::get_ValidFormParent() noexcept
{
    auto parent = this;
    while (parent)
    {
        if (parent->is_FormParent())
        {
            return parent;
        }
        parent = parent->get_Parent();
    }

    return nullptr;
}

bool Node::AdoptChild(NodeSharedPtr child)
{
    ASSERT_MSG(child != get_SharedPtr(), "A node can't adopt itself!");
    if (is_ChildAllowed(child))
    {
        m_children.push_back(child);
        child->set_Parent(get_SharedPtr());
        return true;
    }

    return false;
}

bool Node::AddChild(NodeSharedPtr node)
{
    if (is_ChildAllowed(node))
    {
        m_children.push_back(node);
        return true;
    }

    return false;
}

bool Node::AddChild(Node* node)
{
    if (is_ChildAllowed(node))
    {
        m_children.push_back(node->get_SharedPtr());
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, NodeSharedPtr node)
{
    if (is_ChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node);
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, Node* node)
{
    if (is_ChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node->get_SharedPtr());
        return true;
    }

    return false;
}

bool Node::is_ChildAllowed(NodeDeclaration* child)
{
    ASSERT(child);

    auto max_children = m_declaration->get_AllowableChildren(child->get_GenType());

    if (max_children == child_count::none)
        return false;

    if (max_children == child_count::infinite)
        return true;

    if (is_Gen(gen_wxSplitterWindow))
        return (get_ChildCount() < 2);

    // Because m_children contains shared_ptrs, we don't want to use an iteration loop which will
    // get/release the shared ptr. Using an index into the vector lets us access the raw pointer.

    ptrdiff_t children = 0;
    for (size_t i = 0; i < m_children.size() && children <= max_children; ++i)
    {
        if (get_Child(i)->get_GenType() == child->get_GenType())
            ++children;

        // treat type-sizer and type_gbsizer as the same since forms and contains can only have one
        // of them as the top level sizer.

        else if (child->get_GenType() == type_sizer && get_Child(i)->get_GenType() == type_gbsizer)
            ++children;
        else if (child->get_GenType() == type_gbsizer && get_Child(i)->get_GenType() == type_sizer)
            ++children;
    }

    if (children >= max_children)
        return false;

    return true;
}

bool Node::is_ChildAllowed(Node* child)
{
    ASSERT(child);

    return is_ChildAllowed(child->get_NodeDeclaration());
}

void Node::RemoveChild(Node* node)
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

void Node::RemoveChild(size_t pos)
{
    ASSERT(pos < m_children.size());

    auto iter = m_children.begin() + pos;
    m_children.erase(iter);
}

size_t Node::get_ChildPosition(Node* node)
{
    for (size_t pos = 0; const auto& child: m_children)
    {
        if (child.get() == node)
        {
            return pos;
        }
        ++pos;
    }

    // REVIEW: [Randalphwa - 06-13-2022] Actually, this is sometimes valid when using undo. What
    // really should happen is that it should return int64_t so that -1 becomes a valid return.

    // FAIL_MSG("failed to find child node, returned position is invalid!")
    return (m_children.size() - 1);
}

bool Node::ChangeChildPosition(NodeSharedPtr node, size_t pos)
{
    size_t cur_pos = get_ChildPosition(node.get());

    if (cur_pos == get_ChildCount() || pos >= get_ChildCount())
        return false;

    if (pos == cur_pos)
        return true;

    RemoveChild(node);
    AddChild(pos, node);
    return true;
}

bool Node::is_Local() const noexcept
{
    return is_PropValue(prop_class_access, "none");
}

bool Node::HasValue(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].HasValue();
    else
        return false;
}

bool Node::is_PropValue(PropName name, const char* value) const noexcept
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_string().is_sameas(value));
    }

    return false;
}

bool Node::is_PropValue(PropName name, bool value) const noexcept
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_bool() == value);
    }

    return false;
}

bool Node::is_PropValue(PropName name, int value) const noexcept
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

tt_string* Node::get_PropValuePtr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_raw_ptr();
    else
        return nullptr;
}

tt_string Node::get_PropId() const
{
    tt_string id;
    if (auto result = m_prop_indices.find(prop_id); result != m_prop_indices.end())
        id = m_properties[result->second].get_PropId();
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

const tt_string& Node::get_PropDefaultValue(PropName name)
{
    auto prop = get_PropPtr(name);

    ASSERT_MSG(prop, tt_string(get_NodeName())
                         << " doesn't have the property " << map_PropNames[name]);

    if (prop)
        return prop->getDefaultValue();
    else
        return tt_empty_cstr;
}

const tt_string& Node::get_NodeName() const
{
    if (auto it = m_prop_indices.find(prop_var_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else if (it = m_prop_indices.find(prop_class_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else
        return tt_empty_cstr;
}

tt_string_view Node::get_NodeName(GenLang lang) const
{
    tt_string_view name = get_NodeName();
    if (name.empty())
        return "unknown node";
    if (lang == GEN_LANG_CPLUSPLUS)
    {
        // Valid for Ruby, but not for C++
        if (name[0] == '@')
            name.remove_prefix(1);
        else if (name[0] == '$')  // commonly used for Perl variables
            name.remove_prefix(1);
        // Used for local Python variables, but non-standard for C++ where '_' is typically used for
        // member variables
        else if (name[0] == '_' && is_Local())
            name.remove_prefix(1);
        return name;
    }

    if (name[0] == '@' && lang != GEN_LANG_RUBY)
    {
        name.remove_prefix(1);
        return name;
    }
    else if (name[0] == '$' && lang != GEN_LANG_PERL)
    {
        name.remove_prefix(1);
        return name;
    }

    // GEN_LANG_CPLUSPLUS is handled above
    ASSERT(lang != GEN_LANG_CPLUSPLUS);
    if (name.starts_with("m_"))
        name.remove_prefix(2);
    return name;
}

const tt_string& Node::get_ParentName() const
{
    if (m_parent)
        return m_parent->get_NodeName();

    return tt_empty_cstr;
}

tt_string_view Node::get_ParentName(GenLang lang, bool ignore_sizers) const
{
    if (ignore_sizers)
    {
        auto parent = get_Parent();
        while (parent && parent->is_Sizer())
        {
            parent = parent->get_Parent();
        }
        if (parent)
            return parent->get_NodeName(lang);
    }
    else
    {
        if (m_parent)
            return m_parent->get_NodeName(lang);
    }
    return tt_empty_cstr;
}

const tt_string& Node::get_FormName()
{
    if (auto form = get_Form(); form)
    {
        return form->get_NodeName();
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
            if (alignment.contains("wxALIGN_CENTER_VERTICAL") &&
                alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
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

std::pair<NodeSharedPtr, int> Node::CreateChildNode(GenName name, bool verify_language_support,
                                                    int pos)
{
    auto& frame = wxGetFrame();

    auto result = NodeCreation.CreateNode(name, this, verify_language_support);
    if (!result.first || result.second < 0)
    {
        return { nullptr, result.second };
    }
    auto new_node = result.first;

    Node* parent = this;

    if (!new_node)
    {
        new_node = NodeCreation.CreateNode(name, this).first;
        if ((is_Form() || is_Container()) && get_ChildCount())
        {
            if (get_Child(0)->get_GenType() == type_sizer ||
                get_Child(0)->get_GenType() == type_gbsizer)
            {
                result = NodeCreation.CreateNode(name, get_Child(0), verify_language_support);
                if (!result.first || result.second < 0)
                {
                    return { nullptr, result.second };
                }
                new_node = result.first;
                parent = get_Child(0);
            }

            if (parent->get_GenType() == type_gbsizer)
            {
                GridBag grid_bag(parent);
                if (grid_bag.InsertNode(parent, new_node.get()))
                    return { new_node, Node::valid_node };
                else
                {
                    return { nullptr, Node::gridbag_insert_error };
                }
            }
        }
    }

    if (new_node)
    {
        if (is_Gen(gen_wxGridBagSizer))
        {
            GridBag grid_bag(this);
            if (grid_bag.InsertNode(this, new_node.get()))
                return { new_node, Node::valid_node };
            else
            {
                return { nullptr, Node::gridbag_insert_error };
            }
        }

#if defined(_WIN32)

        // In a Windows build, the default background colour of white doesn't match the normal
        // background color of the parent so we set it to the more normal Windows colour.

        // REVIEW: [KeyWorks - 03-17-2021] Need to figure out a better way to do this which is cross
        // platform. As it currently exists, if the Windows version of wxUiEditor is used, then ALL
        // versions of the app the user creates will use this background color.

        if (name == gen_BookPage)
        {
            if (auto prop = new_node->get_PropPtr(prop_background_colour); prop)
            {
                prop->set_value("wxSYS_COLOUR_BTNFACE");

                // REVIEW: [Randalphwa - 06-18-2023] The node has not been inserted yet, so
                // firing a prop change event can result in a hang, particularly when
                // CreateChildNode() was called while creating a new Book container.

                // frame.FirePropChangeEvent(prop);
            }
        }
#endif  // _WIN32

        tt_string undo_str;
        undo_str << "insert " << map_GenNames[name];
        frame.PushUndoAction(
            std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    }

    // A "ribbonButton" component is used for both wxRibbonButtonBar and wxRibbonToolBar. If
    // creating the node failed, then assume the parent is wxRibbonToolBar and retry with
    // "ribbonTool"
    else if (name == gen_ribbonButton)
    {
        result = NodeCreation.CreateNode(gen_ribbonTool, this);
        if (!result.first || result.second < 0)
        {
            return { nullptr, result.second };
        }
        new_node = result.first;
        tt_string undo_str = "insert ribbon tool";
        frame.PushUndoAction(
            std::make_shared<InsertNodeAction>(new_node.get(), this, undo_str, pos));
    }
    else
    {
        // If we get here, then the current node cannot be a parent of the new node. We look at the
        // current node's parent and see if it accepts the new node as a child. This deals with the
        // case where the user selected a widget, and wants a new widget created right after the
        // selected widget with both having the same parent (typically a sizer).

        parent = get_Parent();

        if (parent)
        {
            auto decl = NodeCreation.get_declaration(name);
            auto max_children = get_NodeDeclaration()->get_AllowableChildren(decl->get_GenType());
            auto cur_children = NodeCreation.CountChildrenWithSameType(this, decl->get_GenType());
            if (max_children > 0 && cur_children >= static_cast<size_t>(max_children))
            {
                if (is_Gen(gen_wxSplitterWindow))
                {
                    wxMessageBox("You cannot add more than two windows to a splitter window.",
                                 "Cannot add control");
                }
                else
                {
                    wxMessageBox(tt_string()
                                 << "You can only add " << (to_size_t) max_children << ' '
                                 << map_GenNames[name] << " as a child of " << get_DeclName());
                }

                return { nullptr, Node::invalid_child_count };
            }

            new_node = NodeCreation.CreateNode(name, parent).first;
            if (new_node)
            {
                if (parent->is_Gen(gen_wxGridBagSizer))
                {
                    GridBag grid_bag(parent);
                    if (grid_bag.InsertNode(parent, new_node.get()))
                        return { new_node, Node::valid_node };
                    else
                    {
                        return { nullptr, Node::gridbag_insert_error };
                    }
                }

                auto insert_pos = parent->FindInsertionPos(this);
                tt_string undo_str;
                undo_str << "insert " << map_GenNames[name];
                frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent,
                                                                        undo_str, insert_pos));
            }
        }
        else
        {
            wxMessageBox(tt_string() << "You cannot add " << map_GenNames[name] << " as a child of "
                                     << get_DeclName());
            return { nullptr, Node::invalid_child };
        }
    }

    if (new_node)
    {
        bool is_name_changed = false;
        if (Project.get_CodePreference(this) == GEN_LANG_CPLUSPLUS)
        {
            if (new_node->HasProp(prop_var_name) && UserPrefs.is_CppSnakeCase())
            {
                auto member_name = ConvertToSnakeCase(new_node->as_string(prop_var_name));
                new_node->set_value(prop_var_name, member_name);
                new_node->FixDuplicateName();
            }
        }
        else
        {
            tt_string member_name = new_node->as_string(prop_var_name);
            if (Project.get_CodePreference(this) == GEN_LANG_RUBY ||
                Project.get_CodePreference(this) == GEN_LANG_PYTHON)
            {
                member_name = ConvertToSnakeCase(member_name);
                if (member_name != new_node->as_string(prop_var_name))
                    is_name_changed = true;
            }

            if (member_name.starts_with("m_"))
            {
                if (Project.get_CodePreference(this) == GEN_LANG_PYTHON)
                {
                    // Python public names don't have a prefix
                    member_name.erase(0, 2);
                }
                else if (Project.get_CodePreference(this) == GEN_LANG_RUBY)
                {
                    // We don't add the '@' because that will be added automatically during
                    // code generation.
                    member_name.erase(0, 2);
                }

                if (member_name.ends_with("_2"))
                {
                    // This is unlikely, but the previous check for duplication assumed a m_
                    // prefix, so without the prefix, it's possible that the name isn't a
                    // duplicate. We only check for _2 since a mix of names with/without a m_
                    // prefix is unlikely.
                    member_name.erase(member_name.size() - 2);
                }

                is_name_changed = true;
            }
            else if (Project.get_CodePreference(this) == GEN_LANG_PYTHON)
            {
                // Python private names have '_' as a prefix
                member_name.insert(0, "_");
                is_name_changed = true;
            }

            if (is_name_changed)
            {
                new_node->set_value(prop_var_name, member_name);
                new_node->FixDuplicateName();
            }
        }

        frame.FireCreatedEvent(new_node.get());
        frame.SelectNode(new_node.get(), evt_flags::fire_event | evt_flags::force_selection);
    }
    return { new_node, Node::valid_node };
}

Node* Node::CreateNode(GenName name)
{
    auto& frame = wxGetFrame();
    auto cur_selection = frame.getSelectedNode();
    if (!cur_selection)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return nullptr;
    }
    return cur_selection->CreateChildNode(name).first.get();
}

void Node::ModifyProperty(PropName name, tt_string_view value)
{
    auto prop = get_PropPtr(name);
    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(tt_string_view name, int value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = get_PropPtr(find_prop->second);

    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(tt_string_view name, tt_string_view value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = get_PropPtr(find_prop->second);

    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(NodeProperty* prop, int value)
{
    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

tt_string Node::get_UniqueName(const tt_string& proposed_name, PropName prop_name)
{
    tt_string new_name(proposed_name);
    if (is_Form())
        return {};

    auto form = get_Form();
    if (!form)
        return {};

    std::unordered_set<std::string> name_set;

    if (prop_name == prop_var_name)
    {
        for (auto& iter: reserved_names)
        {
            name_set.emplace(iter);
        }

        form->CollectUniqueNames(name_set, this, prop_name);
    }
    else if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        if (auto parent = get_Parent(); parent)
        {
            if (parent->is_Gen(gen_propGridPage))
            {
                parent = parent->get_Parent();
            }
            if (parent)
            {
                parent->CollectUniqueNames(name_set, this, prop_name);
            }
        }
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
            // new_name << org_name << '_' << i;
            new_name << org_name << i;
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

bool Node::FixDuplicateName()
{
    if (is_Type(type_form) || is_Type(type_frame_form) || is_Type(type_menubar_form) ||
        is_Type(type_ribbonbar_form) || is_Type(type_toolbar_form) ||
        is_Type(type_aui_toolbar_form) || is_Type(type_panel_form) || is_Type(type_wizard) ||
        is_Type(type_popup_menu) || is_Type(type_project))
    {
        return false;
    }

    auto form = get_Form();
    ASSERT(form || is_Folder());
    if (!form)
        return false;

    std::unordered_set<std::string> name_set;
    for (auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }

    form->CollectUniqueNames(name_set, this);

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
                    // new_name << org_name << '_' << i;
                    new_name << org_name << i;
                }

                auto fix_name = get_PropPtr(iter);
                fix_name->set_value(new_name);
                replaced = true;
            }
        }
    }

    if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        name_set.clear();
        form->CollectUniqueNames(name_set, this, prop_label);

        tt_string org_name(as_string(prop_label));
        auto result = get_UniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto fix_name = get_PropPtr(prop_label);
            fix_name->set_value(result);
        }
    }

    return replaced;
}

void Node::FixDuplicateNodeNames(Node* form)
{
    if (!form)
    {
        if (is_Form())
        {
            for (auto& child: get_ChildNodePtrs())
            {
                child->FixDuplicateNodeNames(this);
            }
            return;
        }
        else
        {
            form = get_Form();
        }

        ASSERT(form);
        if (!form)
            return;
    }

    // Collect all variable names except for those in the current node. Then check if any of
    // the variable names in the current node are duplicated in other nodes, and if so fix
    // them. Then step into each of the child nodes recursively until all nodes have been
    // processed. The reason for collecting the names for each set is because there could be
    // multiple identical names if the node was generated by one of the CreateNew functions.
    // Even pasting in from wxSmith or wxFormBuilder could have multiple identical names if
    // they didn't do their own name fixups correctly.

    std::unordered_set<std::string> name_set;
    for (auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }

    form->CollectUniqueNames(name_set, this);

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
                    // new_name << org_name << '_' << i;
                    new_name << org_name << i;
                }

                auto fix_name = get_PropPtr(iter);
                fix_name->set_value(new_name);
            }
        }
    }

    for (const auto& child: get_ChildNodePtrs())
    {
        child->FixDuplicateNodeNames(form);
    }

    if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        name_set.clear();
        if (auto parent = get_Parent(); parent)
        {
            if (parent && parent->is_Gen(gen_propGridPage))
            {
                parent = parent->get_Parent();
            }

            if (parent)
            {
                parent->CollectUniqueNames(name_set, this, prop_label);
            }
        }

        tt_string org_name(as_string(prop_label));
        auto result = get_UniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto fix_name = get_PropPtr(prop_label);
            fix_name->set_value(result);
        }
    }
}

void Node::CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node,
                              PropName prop_name)
{
    if (!is_Form() && cur_node != this && !is_Gen(gen_wxPropertyGrid) &&
        !is_Gen(gen_wxPropertyGridManager))
    {
        if (prop_name == prop_var_name)
        {
            for (auto& iter: s_var_names)
            {
                // First check the parent node
                if (auto& name = as_string(iter); name.size())
                {
                    name_set.emplace(name);
                }

                // Now check the child node, skipping prop_var_name. This is needed in order to
                // pick up validator names.
                if (iter != prop_var_name)
                {
                    if (auto& name = cur_node->as_string(iter); name.size())
                    {
                        name_set.emplace(name);
                    }
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

    for (const auto& iter: get_ChildNodePtrs())
    {
        iter->CollectUniqueNames(name_set, cur_node, prop_name);
    }
}

ptrdiff_t Node::FindInsertionPos(Node* child) const
{
    if (child)
    {
        for (size_t new_pos = 0; new_pos < get_ChildCount(); ++new_pos)
        {
            if (get_Child(new_pos) == child)
                return new_pos + 1;
        }
    }
    return -1;
}

size_t Node::get_NodeSize() const
{
    auto size = sizeof(*this);
    // Add the size of all the node pointers, but not the size of the individual children
    size += (m_children.size() * sizeof(void*));

    for (auto& iter: m_properties)
    {
        size += iter.get_PropSize();
    }

    for (auto& iter: m_map_events)
    {
        size += iter.second.get_EventSize();
    }

    size += (m_prop_indices.size() * (sizeof(size_t) * 2));

    return size;
}

// Create a hash of the node name and all property values of the node, and recursively call all
// children
void Node::CalcNodeHash(size_t& hash) const
{
    // djb2 hash algorithm

    if (hash == 0)
        hash = 5381;

    for (auto iter: get_NodeName())
        hash = ((hash << 5) + hash) ^ iter;

    for (auto prop: m_properties)
    {
        for (auto char_iter: prop.as_string())
            hash = ((hash << 5) + hash) ^ char_iter;
    }

    for (auto child: m_children)
    {
        child->CalcNodeHash(hash);
    }
}

std::vector<NodeProperty*> Node::FindAllChildProperties(PropName name)
{
    std::vector<NodeProperty*> result;

    FindAllChildProperties(result, name);

    return result;
}

void Node::FindAllChildProperties(std::vector<NodeProperty*>& list, PropName name)
{
    for (const auto& child: m_children)
    {
        if (child->HasValue(name))
        {
            list.emplace_back(child->get_PropPtr(name));
        }
        if (child->get_ChildCount())
        {
            child->FindAllChildProperties(list, name);
        }
    }
}

// clang-format off

static std::unordered_set<GenEnum::GenName> s_bool_validators = {

    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_wxCheckBox,
    gen_wxRadioButton,
    gen_wxToggleButton,
};

static std::unordered_set<GenEnum::GenName> s_int_validators = {

    gen_wxChoice,
    gen_wxGauge,
    gen_wxRadioBox,
    gen_wxScrollBar,
    gen_wxSlider,
    gen_wxSpinButton,
    // REVIEW: [Randalphwa - 07-31-2023] We list this as an int data type, but it's not listed
    // at all in valgen.h -- seems more likely that it is a wxString type if used at all
    //
    gen_wxSpinCtrl,

};

// These are for a read-only wxString
static std::unordered_set<GenEnum::GenName> s_read_only_validators = {

    gen_wxButton,
    gen_wxComboBox,  // get should still work, unlike button and static text
    gen_wxStaticText,
    gen_wxTextCtrl,

};

// These use a wxArrayInt variable
static std::unordered_set<GenEnum::GenName> s_array_int_validators = {

    gen_wxCheckListBox,
    gen_wxListBox,

};

// These use a wxDatePickerCtrl variable
static std::unordered_set<GenEnum::GenName> s_date_picker_validators = {

    gen_wxDatePickerCtrl,

};

// clang-format on

tt_string Node::get_ValidatorDataType() const
{
    tt_string data_type;
    if (HasProp(prop_validator_data_type))
    {
        data_type = as_string(prop_validator_data_type);
    }
    else
    {
        if (s_bool_validators.contains(get_GenName()))
        {
            data_type = "bool";
        }
        else if (s_int_validators.contains(get_GenName()))
        {
            data_type = "int";
        }
        else if (s_read_only_validators.contains(get_GenName()))
        {
            data_type = "wxString";
        }
        else if (s_array_int_validators.contains(get_GenName()))
        {
            data_type = "wxArrayInt";
        }
        else if (s_date_picker_validators.contains(get_GenName()))
        {
            data_type = "wxDatePickerCtrl";
        }
    }

    return data_type;
}

tt_string_view Node::get_ValidatorType() const
{
    if (!is_Gen(gen_wxTextCtrl))
    {
        return "wxGenericValidator";
    }
    else
    {
        auto& data_type = as_string(prop_validator_data_type);
        if (data_type == "wxString")
            return "wxTextValidator";
        else if (data_type == "int" || data_type == "short" || data_type == "long" ||
                 data_type == "long long" || data_type == "unsigned int" ||
                 data_type == "unsigned short" || data_type == "unsigned long" ||
                 data_type == "unsigned long long")
            return "wxIntegerValidator";
        else if (data_type == "double" || data_type == "float")
            return "wxFloatingPointValidator";
        else
            return "wxGenericValidator";
    }
}

Node* Node::get_PlatformContainer()
{
    if (auto parent = get_Parent(); parent)
    {
        do
        {
            if (parent->HasProp(prop_platforms) &&
                parent->as_string(prop_platforms) != "Windows|Unix|Mac")
                return parent;
            parent = parent->get_Parent();
        } while (parent && !parent->is_Gen(gen_Project));
    }
    return nullptr;
}

std::vector<tt_string>* Node::get_InternalData()
{
    if (!m_internal_data)
        m_internal_data = std::make_unique<std::vector<tt_string>>();
    return m_internal_data.get();
}
