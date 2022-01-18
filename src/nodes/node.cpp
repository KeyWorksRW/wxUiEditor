/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains user-modifiable node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/sizer.h>    // provide wxSizer class for layout

#include "node.h"

#include "appoptions.h"    // AppOptions -- Application-wide options
#include "gridbag_item.h"  // GridBagItem -- Dialog for inserting an item into a wxGridBagSizer node
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node_creator.h"  // NodeCreator class
#include "node_decl.h"     // NodeDeclaration class
#include "node_gridbag.h"  // GridBag -- Create and modify a node containing a wxGridBagSizer
#include "node_prop.h"     // NodeProperty -- NodeProperty class
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction

using namespace GenEnum;

// clang-format off
inline const GenType lst_form_types[] =
{

    type_form,
    type_frame_form,
    type_images,
    type_menubar_form,
    type_popup_menu,
    type_ribbonbar_form,
    type_toolbar_form,
    type_wizard,

};
// clang-format on

bool Node::IsForm() const noexcept
{
    for (auto& iter: lst_form_types)
    {
        if (isType(iter))
            return true;
    }
    return false;
}

// Same as wxGetApp() only this returns a reference to the project node
Node& wxGetProject()
{
    ASSERT_MSG(wxGetApp().GetProjectPtr(), "MainFrame hasn't been created yet.");
    return *wxGetApp().GetProjectPtr().get();
}

Node::Node(NodeDeclaration* declaration) : m_declaration(declaration) {}

NodeProperty* Node::get_prop_ptr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return &m_properties[result->second];
    else
        return nullptr;
}

NodeEvent* Node::GetEvent(ttlib::cview name)
{
    if (auto it = m_event_map.find(name.c_str()); it != m_event_map.end())
        return &m_events[it->second];
    else
        return nullptr;
}

NodeEvent* Node::GetEvent(size_t index)
{
    ASSERT(index < m_events.size());
    return &m_events[index];
}

size_t Node::GetInUseEventCount() const
{
    size_t count = 0;

    for (auto& iter: m_events)
    {
        if (iter.get_value().size())
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

NodeEvent* Node::AddNodeEvent(const NodeEventInfo* info)
{
    auto& event = m_events.emplace_back(info, this);
    m_event_map[event.get_name()] = (m_events.size() - 1);
    return &m_events[m_events.size() - 1];
}

Node* Node::LocateAncestorType(GenType type) const noexcept
{
    auto parent = GetParent();
    while (parent && !parent->isType(type))
    {
        parent = parent->GetParent();
    }

    return parent;
}

Node* Node::FindParentForm() const noexcept
{
    for (auto& iter: lst_form_types)
    {
        if (auto retObj = LocateAncestorType(iter); retObj)
            return retObj;
    }

    return nullptr;
}

bool Node::Adopt(NodeSharedPtr child)
{
    ASSERT_MSG(child != GetSharedPtr(), "A node can't adopt itself!");
    if (IsChildAllowed(child))
    {
        m_children.push_back(child);
        child->SetParent(GetSharedPtr());
        return true;
    }

    return false;
}

bool Node::AddChild(NodeSharedPtr node)
{
    if (IsChildAllowed(node))
    {
        m_children.push_back(node);
        return true;
    }

    return false;
}

bool Node::AddChild(Node* node)
{
    if (IsChildAllowed(node))
    {
        m_children.push_back(node->GetSharedPtr());
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, NodeSharedPtr node)
{
    if (IsChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node);
        return true;
    }

    return false;
}

bool Node::AddChild(size_t idx, Node* node)
{
    if (IsChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + idx, node->GetSharedPtr());
        return true;
    }

    return false;
}

bool Node::IsChildAllowed(Node* child)
{
    ASSERT(child);

    auto child_type = child->gen_type();
    auto max_children = m_declaration->GetAllowableChildren(child_type);

    if (max_children == child_count::none)
        return false;

    if (max_children == child_count::infinite)
        return true;

    if (isGen(gen_wxSplitterWindow))
        return (GetChildCount() < 2);

    // Because m_children contains shared_ptrs, we don't want to use an iteration loop which will get/release the shared
    // ptr. Using an index into the vector lets us access the raw pointer.

    int_t children = 0;
    for (size_t i = 0; i < m_children.size() && children <= max_children; ++i)
    {
        if (GetChild(i)->gen_type() == child_type)
            ++children;
    }

    if (children >= max_children)
        return false;

    return true;
}

void Node::RemoveChild(NodeSharedPtr node)
{
    auto iter = m_children.begin();
    while (iter != m_children.end() && iter->get() != node.get())
        iter++;

    if (iter != m_children.end())
        m_children.erase(iter);
}

void Node::RemoveChild(Node* node)
{
    auto iter = m_children.begin();
    while (iter != m_children.end() && iter->get() != node)
        iter++;

    if (iter != m_children.end())
        m_children.erase(iter);
}

void Node::RemoveChild(size_t pos)
{
    ASSERT(pos < m_children.size());

    auto iter = m_children.begin() + pos;
    m_children.erase(iter);
}

size_t Node::GetChildPosition(Node* node)
{
    size_t pos = 0;
    while (pos < GetChildCount() && m_children[pos].get() != node)
        ++pos;

    return pos;
}

bool Node::ChangeChildPosition(NodeSharedPtr node, size_t pos)
{
    size_t cur_pos = GetChildPosition(node.get());

    if (cur_pos == GetChildCount() || pos >= GetChildCount())
        return false;

    if (pos == cur_pos)
        return true;

    RemoveChild(node.get());
    AddChild(pos, node);
    return true;
}

bool Node::IsLocal() const noexcept
{
    return isPropValue(prop_class_access, "none");
}

bool Node::HasValue(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].HasValue();
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

bool Node::prop_as_bool(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_bool();
    else
        return false;
}

int Node::prop_as_int(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_int();
    else
        return 0;
}

int Node::prop_as_mockup(PropName name, std::string_view prefix) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_mockup(prefix);
    else
        return 0;
}

wxColour Node::prop_as_wxColour(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_color();
    else
        return wxColour();
}

wxFont Node::prop_as_font(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_font();
    else
        return *wxNORMAL_FONT;
}

wxPoint Node::prop_as_wxPoint(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_point();
    else
        return wxDefaultPosition;
}

wxSize Node::prop_as_wxSize(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_size();
    else
        return wxDefaultSize;
}

wxAnimation Node::prop_as_wxAnimation(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_animation();
    else
        return wxNullAnimation;
}

wxBitmap Node::prop_as_wxBitmap(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_bitmap();
    else
        return wxNullBitmap;
}

wxArrayString Node::prop_as_wxArrayString(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_wxArrayString();
    else
        return wxArrayString();
}

FontProperty Node::prop_as_font_prop(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_font_prop();
    else
        return FontProperty(wxNORMAL_FONT);
}

double Node::prop_as_double(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_float();
    else
        return 0;
}

wxString Node::prop_as_wxString(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_wxString();
    else
        return wxString();
}

const ttlib::cstr& Node::prop_as_string(PropName name) const
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_string();
    else
        return tt_empty_cstr;
}

const ttlib::cstr& Node::prop_as_constant(PropName name, std::string_view prefix)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_constant(prefix);
    else
        return tt_empty_cstr;
}

ttlib::cstr* Node::prop_as_raw_ptr(PropName name)
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
        return m_properties[result->second].as_raw_ptr();
    else
        return nullptr;
}

const ttlib::cstr& Node::prop_default_value(PropName name)
{
    auto prop = get_prop_ptr(name);

    ASSERT_MSG(prop, ttlib::cstr(get_node_name()) << " doesn't have the property " << map_PropNames[name]);

    if (prop)
        return prop->GetDefaultValue();
    else
        return tt_empty_cstr;
}

const ttlib::cstr& Node::get_node_name() const
{
    if (auto it = m_prop_indices.find(prop_var_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else if (it = m_prop_indices.find(prop_class_name); it != m_prop_indices.end())
        return m_properties[it->second].as_string();
    else
        return tt_empty_cstr;
}

const ttlib::cstr& Node::get_parent_name() const
{
    if (m_parent)
        return m_parent->get_node_name();

    return tt_empty_cstr;
}

const ttlib::cstr& Node::get_form_name() const
{
    if (auto form = FindParentForm(); form)
    {
        return form->get_node_name();
    }
    return tt_empty_cstr;
}

wxSizerFlags Node::GetSizerFlags() const
{
    wxSizerFlags flags;
    flags.Proportion(prop_as_int(prop_proportion));
    auto border_size = prop_as_int(prop_border_size);
    int direction = 0;
    auto& border_settings = prop_as_string(prop_borders);
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

    if (auto& alignment = prop_as_string(prop_alignment); alignment.size())
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

    if (auto& prop = prop_as_string(prop_flags); prop.size())
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

Node* Node::CreateChildNode(GenName name)
{
    auto& frame = wxGetFrame();

    auto new_node = g_NodeCreator.CreateNode(name, this);

    Node* parent = this;

    if (!new_node)
    {
        if ((IsForm() || IsContainer()) && GetChildCount() && GetChild(0)->isGen(gen_wxBoxSizer))
        {
            new_node = g_NodeCreator.CreateNode(name, GetChild(0));
            if (!new_node)
                return nullptr;
            parent = GetChild(0);
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
            if (auto prop = new_node->get_prop_ptr(prop_background_colour); prop)
            {
                prop->set_value("wxSYS_COLOUR_BTNFACE");
                frame.FirePropChangeEvent(prop);
            }
        }
#endif  // _WIN32

        ttlib::cstr undo_str;
        undo_str << "insert " << map_GenNames[name];
        frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str));
    }

    // A "ribbonButton" component is used for both wxRibbonButtonBar and wxRibbonToolBar. If creating the node failed,
    // then assume the parent is wxRibbonToolBar and retry with "ribbonTool"
    else if (name == gen_ribbonButton)
    {
        new_node = g_NodeCreator.CreateNode(gen_ribbonTool, this);
        if (new_node)
        {
            ttlib::cstr undo_str = "insert ribbon tool";
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

        parent = GetParent();

        if (parent)
        {
            auto decl = g_NodeCreator.get_declaration(name);
            auto max_children = GetNodeDeclaration()->GetAllowableChildren(decl->gen_type());
            auto cur_children = g_NodeCreator.CountChildrenWithSameType(this, decl->gen_type());
            if (max_children > 0 && cur_children >= static_cast<size_t>(max_children))
            {
                if (isGen(gen_wxSplitterWindow))
                {
                    wxMessageBox("You cannot add more than two windows to a splitter window.", "Cannot add control");
                }
                else
                {
                    wxMessageBox(ttlib::cstr() << "You can only add " << static_cast<size_t>(max_children) << ' '
                                               << map_GenNames[name] << " as a child of " << DeclName());
                }

                return nullptr;
            }

            new_node = g_NodeCreator.CreateNode(name, parent);
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

                auto pos = parent->FindInsertionPos(this);
                ttlib::cstr undo_str;
                undo_str << "insert " << map_GenNames[name];
                frame.PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
            }
        }
        else
        {
            wxMessageBox(ttlib::cstr() << "You cannot add " << map_GenNames[name] << " as a child of " << DeclName());
            return nullptr;
        }
    }

    if (new_node)
    {
        frame.FireCreatedEvent(new_node.get());
        frame.SelectNode(new_node.get(), true, true);
    }
    return new_node.get();
}

Node* Node::CreateNode(GenName name)
{
    auto& frame = wxGetFrame();
    auto cur_selection = frame.GetSelectedNode();
    if (!cur_selection)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return nullptr;
    }
    return cur_selection->CreateChildNode(name);
}

void Node::ModifyProperty(PropName name, ttlib::cview value)
{
    auto prop = get_prop_ptr(name);
    if (prop && value != prop->as_cview())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(ttlib::cview name, int value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = get_prop_ptr(find_prop->second);

    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

void Node::ModifyProperty(ttlib::cview name, ttlib::cview value)
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
        prop = get_prop_ptr(find_prop->second);

    if (prop && value != prop->as_cview())
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

void Node::ModifyProperty(NodeProperty* prop, ttlib::cview value)
{
    if (prop && value != prop->as_cview())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

ttlib::cstr Node::GetUniqueName(const ttlib::cstr& proposed_name)
{
    ttlib::cstr new_name(proposed_name);
    if (IsForm())
        return {};

    auto form = FindParentForm();
    if (!form)
        return {};

    std::unordered_set<std::string> name_set;
    form->CollectUniqueNames(name_set, this);

    if (auto it = name_set.find(new_name); it != name_set.end())
    {
        // We get here if the name has already been used.

        std::string org_name(proposed_name);
        while (ttlib::is_digit(org_name.back()))
        {
            // remove any trailing digits
            org_name.erase(org_name.size() - 1, 1);
        }

        for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
        {
            new_name.clear();
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

bool Node::FixDuplicateName()
{
    if (isType(type_form) || isType(type_frame_form) || isType(type_menubar_form) || isType(type_ribbonbar_form) ||
        isType(type_toolbar_form) || isType(type_wizard) || isType(type_popup_menu) || isType(type_project))
    {
        return false;
    }

    auto form = FindParentForm();
    ASSERT(form);
    if (!form)
        return false;

    std::unordered_set<std::string> name_set;
    form->CollectUniqueNames(name_set, this);

    bool replaced = false;
    for (auto& iter: s_var_names)
    {
        if (auto& name = prop_as_string(iter); name.size())
        {
            if (auto it = name_set.find(name); it != name_set.end())
            {
                // We get here if the name has already been used.

                std::string org_name(name);
                while (ttlib::is_digit(org_name.back()))
                {
                    // remove any trailing digits
                    org_name.erase(org_name.size() - 1, 1);
                }
                if (org_name.back() == '_')
                    org_name.erase(org_name.size() - 1, 1);

                ttlib::cstr new_name;
                for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
                {
                    new_name.clear();
                    new_name << org_name << '_' << i;
                }

                auto fix_name = get_prop_ptr(iter);
                fix_name->set_value(new_name);
                replaced = true;
            }
        }
    }

    return replaced;
}

void Node::FixDuplicateNodeNames(Node* form)
{
    if (!form)
    {
        if (IsForm())
        {
            for (auto& child: GetChildNodePtrs())
            {
                child->FixDuplicateNodeNames(this);
            }
            return;
        }
        else
        {
            form = FindParentForm();
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
    form->CollectUniqueNames(name_set, this);

    for (auto& iter: s_var_names)
    {
        if (auto& name = prop_as_string(iter); name.size())
        {
            if (auto it = name_set.find(name); it != name_set.end())
            {
                // We get here if the name has already been used.

                std::string org_name(name);
                while (ttlib::is_digit(org_name.back()))
                {
                    // remove any trailing digits
                    org_name.erase(org_name.size() - 1, 1);
                }
                if (org_name.back() == '_')
                    org_name.erase(org_name.size() - 1, 1);

                ttlib::cstr new_name;
                for (int i = 2; it != name_set.end(); it = name_set.find(new_name), ++i)
                {
                    new_name.clear();
                    new_name << org_name << '_' << i;
                }

                auto fix_name = get_prop_ptr(iter);
                fix_name->set_value(new_name);
            }
        }
    }

    for (auto& child: GetChildNodePtrs())
    {
        child->FixDuplicateNodeNames(form);
    }
}

void Node::CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node)
{
    if (!IsForm() && cur_node != this)
    {
        for (auto& iter: s_var_names)
        {
            if (auto& name = prop_as_string(iter); name.size())
            {
                name_set.emplace(name);
            }
        }
    }

    for (auto& iter: GetChildNodePtrs())
    {
        iter->CollectUniqueNames(name_set, cur_node);
    }
}

int_t Node::FindInsertionPos(Node* child) const
{
    if (child)
    {
        for (size_t new_pos = 0; new_pos < GetChildCount(); ++new_pos)
        {
            if (GetChild(new_pos) == child)
                return new_pos + 1;
        }
    }
    return -1;
}

size_t Node::GetNodeSize() const
{
    auto size = sizeof(*this);
    // Add the size of all the node pointers, but not the size of the individual children
    size += (m_children.size() * sizeof(void*));

    for (auto& iter: m_properties)
    {
        size += iter.GetPropSize();
    }

    for (auto& iter: m_events)
    {
        size += iter.GetEventSize();
    }

    // Add the size of our maps

    size += (m_prop_indices.size() * (sizeof(size_t) * 2));
    size += (m_event_map.size() * (sizeof(std::string) + sizeof(size_t)));

    return size;
}

// Create a hash of the node name and all property values of the node, and recursively call all children
void Node::CalcNodeHash(size_t& hash) const
{
    // djb2 hash algorithm

    if (hash == 0)
        hash = 5381;

    for (auto iter: get_node_name())
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
    for (size_t idx = 0; idx < m_children.size(); ++idx)
    {
        if (m_children[idx]->HasValue(name))
        {
            list.emplace_back(m_children[idx]->get_prop_ptr(name));
        }
        if (m_children[idx]->GetChildCount())
        {
            m_children[idx]->FindAllChildProperties(list, name);
        }
    }
}
