/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "undo_cmds.h"

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "image_handler.h"        // ImageHandler class, ProjectImages global
#include "mainframe.h"            // MainFrame -- Main window frame
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "node_gridbag.h"         // GridBag -- Create and modify a node containing a wxGridBagSizer
#include "project_handler.h"      // ProjectHandler class
#include "utils.h"                // Utility functions that work with properties

///////////////////////////////// InsertNodeAction ////////////////////////////////////

InsertNodeAction::InsertNodeAction(Node* node, Node* parent, const tt_string& undo_str, int pos)
{
    Init(node->get_SharedPtr(), parent->get_SharedPtr(), undo_str, pos);
}

InsertNodeAction::InsertNodeAction(const NodeSharedPtr node, const NodeSharedPtr parent,
                                   tt_string_view undo_str, int pos)
{
    Init(node, parent, undo_str, pos);
}

void InsertNodeAction::Init(const NodeSharedPtr node, const NodeSharedPtr parent,
                            tt_string_view undo_str, int pos)
{
    m_old_selected = wxGetFrame().getSelectedNodePtr();
    m_node = node;
    m_parent = parent;
    SetUndoString(undo_str);
    m_pos = pos;

    if (m_node->is_Gen(gen_folder) || m_node->is_Gen(gen_sub_folder))
    {
        m_fix_duplicate_names = false;
    }
}

void InsertNodeAction::Change()
{
    m_node->set_Parent(m_parent);

    ASSERT_MSG(!m_parent->is_Gen(gen_wxGridBagSizer),
               "Only use AppendGridBagAction to add items to a wxGridBagSizer!");

    if (m_pos == -1 && m_parent->is_Sizer() && m_parent->get_ChildCount() > 0 &&
        m_parent->get_ChildPtr(m_parent->get_ChildCount() - 1)->is_Gen(gen_wxStdDialogButtonSizer))
    {
        m_parent->AddChild(m_node);
        // Add the child BEFORE any wxStdDialogButtonSizer
        m_parent->ChangeChildPosition(m_node, m_parent->get_ChildCount() - 2);
    }
    else
    {
        m_parent->AddChild(m_node);
        if (m_pos >= 0)
            m_parent->ChangeChildPosition(m_node, m_pos);
    }

    if (m_fix_duplicate_names)
    {
        // This needs to be done only once, even if the insertion is reverted and then changed
        // again. The reason is that any name changes to other nodes cannot be undone.

        m_node->FixDuplicateNodeNames();
        m_fix_duplicate_names = false;
    }

    if (m_fire_created_event)
    {
        wxGetFrame().FireCreatedEvent(m_node.get());
    }

    // Probably not necessary, but with both parameters set to false, this simply ensures the
    // mainframe has it's selection node set correctly.
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node.get(), evt_flags::no_event);
}

void InsertNodeAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());  // Remove the parent pointer
    if (m_fire_created_event)
    {
        wxGetFrame().FireDeletedEvent(m_node.get());
    }
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_old_selected.get());
}

///////////////////////////////// RemoveNodeAction ////////////////////////////////////

RemoveNodeAction::RemoveNodeAction(Node* node, const tt_string& undo_str, bool AddToClipboard)
{
    Init(node->get_SharedPtr(), undo_str, AddToClipboard);
}

RemoveNodeAction::RemoveNodeAction(const NodeSharedPtr node, const tt_string& undo_str,
                                   bool AddToClipboard)
{
    Init(node, undo_str, AddToClipboard);
}

void RemoveNodeAction::Init(const NodeSharedPtr node, tt_string_view undo_str, bool AddToClipboard)
{
    m_AddToClipboard = AddToClipboard;
    m_node = node;
    m_parent = node->get_ParentPtr();
    m_old_pos = m_parent->get_ChildPosition(node);
    m_old_selected = wxGetFrame().getSelectedNodePtr();
    SetUndoString(undo_str);

    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
}

void RemoveNodeAction::Change()
{
    if (m_AddToClipboard)
        wxGetFrame().CopyNode(m_node.get());

    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());

    wxGetFrame().FireDeletedEvent(m_node.get());

    if (m_parent->get_ChildCount())
    {
        auto pos =
            (m_old_pos < m_parent->get_ChildCount() ? m_old_pos : m_parent->get_ChildCount() - 1);
        if (isAllowedSelectEvent())
            wxGetFrame().SelectNode(m_parent->get_Child(pos));
    }
    else
    {
        if (isAllowedSelectEvent())
            wxGetFrame().SelectNode(m_parent.get());
    }
}

void RemoveNodeAction::Revert()
{
    m_parent->AddChild(m_node);
    m_node->set_Parent(m_parent);
    m_parent->ChangeChildPosition(m_node, m_old_pos);

    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_old_selected.get(), evt_flags::force_selection);
}

///////////////////////////////// ModifyPropertyAction ////////////////////////////////////

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, tt_string_view value) :
    m_property(prop)
{
    m_undo_string << "change " << prop->get_DeclName();

    m_change_value << value;
    m_revert_value = prop->as_string();
    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, int value) : m_property(prop)
{
    m_undo_string << "change " << prop->get_DeclName();

    m_change_value << value;
    m_revert_value = prop->as_string();
}

void ModifyPropertyAction::Change()
{
    m_property->set_value(m_change_value);

    wxGetFrame().FirePropChangeEvent(m_property);
}

void ModifyPropertyAction::Revert()
{
    m_property->set_value(m_revert_value);

    wxGetFrame().FirePropChangeEvent(m_property);
}

///////////////////////////////// ModifyProperties ////////////////////////////////////

ModifyProperties::ModifyProperties(tt_string_view undo_string, bool fire_events) :
    UndoAction(undo_string)
{
    m_fire_events = fire_events;
    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

void ModifyProperties::addProperty(NodeProperty* prop, tt_string_view value)
{
    auto& entry = m_properties.emplace_back();
    entry.property = prop;
    entry.change_value = value;
    entry.revert_value = prop->as_string();
}

void ModifyProperties::addProperty(NodeProperty* prop, int value)
{
    auto& entry = m_properties.emplace_back();
    entry.property = prop;
    entry.change_value << value;
    entry.revert_value = prop->as_string();
}

void ModifyProperties::Change()
{
    for (auto& iter: m_properties)
    {
        iter.property->set_value(iter.change_value);
    }

    if (m_fire_events)
        wxGetFrame().FireMultiPropEvent(this);
}

void ModifyProperties::Revert()
{
    for (auto& iter: m_properties)
    {
        iter.property->set_value(iter.revert_value);
    }

    if (m_fire_events)
        wxGetFrame().FireMultiPropEvent(this);
}

size_t ModifyProperties::GetMemorySize()
{
    size_t total = sizeof(*this) + m_properties.size();
    for (auto& iter: m_properties)
    {
        // The +2 is to account for the trailing zero in each std::string value.
        total += (iter.change_value.size() + iter.revert_value.size() + 2);
    }
    return total;
}

///////////////////////////////// ModifyEventAction ////////////////////////////////////

ModifyEventAction::ModifyEventAction(NodeEvent* event, tt_string_view value) :
    m_event(event), m_change_value(value)
{
    m_undo_string << "change " << event->get_name() << " handler";

    m_revert_value = event->get_value();

    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

void ModifyEventAction::Change()
{
    m_event->set_value(m_change_value);
    wxGetFrame().FireChangeEventHandler(m_event);
}

void ModifyEventAction::Revert()
{
    m_event->set_value(m_revert_value);
    wxGetFrame().FireChangeEventHandler(m_event);
}

///////////////////////////////// ChangePositionAction ////////////////////////////////////

ChangePositionAction::ChangePositionAction(Node* node, size_t position)
{
    Init(node->get_SharedPtr(), position);
}

ChangePositionAction::ChangePositionAction(const NodeSharedPtr node, size_t position)
{
    Init(node, position);
}

void ChangePositionAction::Init(const NodeSharedPtr node, size_t position)
{
    m_node = node;
    m_parent = node->get_ParentPtr();

    m_change_pos = position;
    m_revert_pos = m_parent->get_ChildPosition(node);

    SetUndoString(tt_string() << "change " << node->get_DeclName() << " position");

    m_UndoEventGenerated = true;
    m_RedoEventGenerated = true;
    m_UndoSelectEventGenerated = true;
    m_RedoSelectEventGenerated = true;
}

void ChangePositionAction::Change()
{
    m_parent->ChangeChildPosition(m_node, m_change_pos);
    wxGetFrame().FirePositionChangedEvent(this);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node);
}

void ChangePositionAction::Revert()
{
    m_parent->ChangeChildPosition(m_node, m_revert_pos);
    wxGetFrame().FirePositionChangedEvent(this);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node);
}

///////////////////////////////// ChangeSizerType ////////////////////////////////////

ChangeSizerType::ChangeSizerType(Node* node, GenEnum::GenName new_gen_sizer)
{
    m_undo_string << "change sizer type";

    m_old_node = node->get_SharedPtr();
    m_parent = node->get_ParentPtr();
    m_new_gen_sizer = new_gen_sizer;

    m_node = NodeCreation.NewNode(m_new_gen_sizer);
    ASSERT(m_node);
    if (m_node)
    {
        auto new_name = m_old_node->get_UniqueName(m_node->as_string(prop_var_name), prop_var_name);
        m_node->set_value(prop_var_name, new_name);
        if (m_new_gen_sizer == gen_wxFlexGridSizer &&
            (m_old_node->is_Gen(gen_wxBoxSizer) || m_old_node->is_Gen(gen_VerticalBoxSizer)))
        {
            if (m_old_node->as_string(prop_orientation) == "wxHORIZONTAL")
            {
                m_node->set_value(prop_cols, (to_int) m_old_node->get_ChildCount());
            }
            else
            {
                m_node->set_value(prop_cols, 0);
                m_node->set_value(prop_rows, (to_int) m_old_node->get_ChildCount());
            }
        }

        for (const auto& iter: m_old_node->get_ChildNodePtrs())
        {
            m_node->AdoptChild(NodeCreation.MakeCopy(iter.get()));
        }
    }
}

void ChangeSizerType::Change()
{
    auto pos = m_parent->get_ChildPosition(m_old_node.get());
    m_parent->RemoveChild(m_old_node);
    m_old_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_node);
    m_parent->ChangeChildPosition(m_node, pos);

    wxGetFrame().FireDeletedEvent(m_old_node.get());
    wxGetFrame().FireCreatedEvent(m_node);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node.get());
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_node.get(), true, true);
}

void ChangeSizerType::Revert()
{
    auto pos = m_parent->get_ChildPosition(m_node.get());
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_old_node);
    m_parent->ChangeChildPosition(m_old_node, pos);

    wxGetFrame().FireDeletedEvent(m_node.get());
    wxGetFrame().FireCreatedEvent(m_old_node);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_old_node.get());
}

///////////////////////////////// ChangeNodeType ////////////////////////////////////

static auto lst_common_properties = {

    prop_alignment,
    prop_background_colour,
    prop_border_size,
    prop_borders,
    prop_class_access,
    prop_disabled,
    prop_flags,
    prop_focus,
    prop_font,
    prop_foreground_colour,
    prop_hidden,
    prop_hint,
    prop_label,
    prop_max_size,
    prop_min_size,
    prop_platforms,
    prop_pos,
    prop_proportion,
    prop_size,
    prop_tooltip,
    prop_var_comment,
    prop_variant,
    prop_window_extra_style,
    prop_window_style,
    prop_contents,
    prop_selection_string,
    prop_selection_int,

    prop_validator_variable,
    prop_validator_data_type,
    prop_validator_type,
    prop_validator_style,
    prop_get_function,
    prop_set_function,

};

static void CopyCommonProperties(Node* old_node, Node* new_node)
{
    for (auto prop: lst_common_properties)
    {
        if (new_node->HasProp(prop) && old_node->HasProp(prop))
        {
            new_node->set_value(prop, old_node->as_string(prop));
        }
    }

    if (old_node->HasProp(prop_var_name) && old_node->as_string(prop_class_access) != "none")
    {
        new_node->set_value(prop_var_name, old_node->as_string(prop_var_name));
    }

    if (old_node->HasValue(prop_validator_variable))
    {
        new_node->set_value(prop_validator_variable, old_node->as_string(prop_validator_variable));
        new_node->set_value(prop_validator_data_type,
                            old_node->as_string(prop_validator_data_type));
    }

    if (old_node->is_Gen(gen_wxComboBox) && new_node->is_Gen(gen_wxChoice))
    {
        auto map_old_events = old_node->get_MapEvents();
        if (auto event = map_old_events.find("wxEVT_COMBOBOX"); event != map_old_events.end())
        {
            if (event->second.get_value().size())
            {
                auto* new_event = new_node->get_Event("wxEVT_CHOICE");
                if (new_event)
                {
                    new_event->set_value(event->second.get_value());
                }
            }
        }
    }

    // Copy all identical events that have handlers
    new_node->CopyEventsFrom(old_node);
}

ChangeNodeType::ChangeNodeType(Node* node, GenEnum::GenName new_node)
{
    m_undo_string << "change widget type";

    m_old_node = node->get_SharedPtr();
    m_parent = node->get_ParentPtr();
    m_new_gen_node = new_node;

    m_node = NodeCreation.NewNode(m_new_gen_node);
    ASSERT(m_node);
    if (m_node)
    {
        // If the node type has changed, then we use the new type's default name.
        auto new_name = m_old_node->get_UniqueName(m_node->as_string(prop_var_name), prop_var_name);
        CopyCommonProperties(m_old_node.get(), m_node.get());
        m_node->set_value(prop_var_name, new_name);
        if (m_new_gen_node == gen_wxCheckBox || m_new_gen_node == gen_wxRadioBox)
        {
            m_node->set_value(prop_checked, m_old_node->as_bool(prop_checked));
        }

        for (const auto& iter: m_old_node->get_ChildNodePtrs())
        {
            m_node->AdoptChild(NodeCreation.MakeCopy(iter.get()));
        }
    }
}

void ChangeNodeType::Change()
{
    auto pos = m_parent->get_ChildPosition(m_old_node.get());
    m_parent->RemoveChild(m_old_node);
    m_old_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_node);
    m_parent->ChangeChildPosition(m_node, pos);

    wxGetFrame().FireDeletedEvent(m_old_node.get());
    wxGetFrame().FireCreatedEvent(m_node);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node.get());
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_node.get(), true, true);
}

void ChangeNodeType::Revert()
{
    auto pos = m_parent->get_ChildPosition(m_node.get());
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_old_node);
    m_parent->ChangeChildPosition(m_old_node, pos);

    wxGetFrame().FireDeletedEvent(m_node.get());
    wxGetFrame().FireCreatedEvent(m_old_node);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_old_node.get());
}

///////////////////////////////// ChangeParentAction ////////////////////////////////////

ChangeParentAction::ChangeParentAction(Node* node, Node* parent, int pos)
{
    Init(node->get_SharedPtr(), parent->get_SharedPtr(), pos);
}

ChangeParentAction::ChangeParentAction(const NodeSharedPtr node, const NodeSharedPtr parent,
                                       int pos)
{
    Init(node, parent, pos);
}

void ChangeParentAction::Init(const NodeSharedPtr node, const NodeSharedPtr parent, int pos)
{
    m_node = node;
    m_change_parent = parent;
    m_revert_parent = node->get_ParentPtr();
    m_pos = pos;

    m_revert_position = m_revert_parent->get_ChildPosition(node.get());
    m_revert_row = node->as_int(prop_row);
    m_revert_col = node->as_int(prop_column);

    SetUndoString(tt_string() << "change " << node->get_DeclName() << " parent");

    m_UndoEventGenerated = true;
    m_RedoEventGenerated = true;
    m_UndoSelectEventGenerated = true;
    m_RedoSelectEventGenerated = true;
}

void ChangeParentAction::Change()
{
    m_revert_parent->RemoveChild(m_node);
    wxGetFrame().getNavigationPanel()->DeleteNode(m_node.get());
    if (m_change_parent->is_Gen(gen_wxGridBagSizer))
    {
        GridBag grid_bag(m_change_parent.get());
        if (!grid_bag.InsertNode(m_change_parent.get(), m_node.get()))
        {
            m_node->set_Parent(m_revert_parent);
            m_revert_parent->AddChild(m_node);
            m_revert_parent->ChangeChildPosition(m_node, m_revert_position);
            // Since we deleted it from Navigation Panel, need to add it back
            wxGetFrame().FireParentChangedEvent(this);
            if (isAllowedSelectEvent())
                wxGetFrame().SelectNode(m_node);
        }
    }
    else
    {
        bool result = false;
        if (m_pos >= 0)
            result = m_change_parent->AddChild(static_cast<size_t>(m_pos), m_node);
        else
            result = m_change_parent->AddChild(m_node);
        ASSERT_MSG(result, tt_string("Unable to change parent of ")
                               << m_node->get_NodeName() << " to "
                               << m_change_parent->get_NodeName());
        if (result)
        {
            m_node->set_Parent(m_change_parent);

            wxGetFrame().FireParentChangedEvent(this);
            if (isAllowedSelectEvent())
                wxGetFrame().SelectNode(m_node);
        }
    }
}

void ChangeParentAction::Revert()
{
    m_change_parent->RemoveChild(m_node);

    m_node->set_Parent(m_revert_parent);
    m_revert_parent->AddChild(m_node);
    m_revert_parent->ChangeChildPosition(m_node, m_revert_position);
    if (auto prop = m_node->get_PropPtr(prop_row); prop)
        prop->set_value(m_revert_row);
    if (auto prop = m_node->get_PropPtr(prop_column); prop)
        prop->set_value(m_revert_col);

    wxGetFrame().FireParentChangedEvent(this);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node);
}

///////////////////////////////// AppendGridBagAction ////////////////////////////////////

AppendGridBagAction::AppendGridBagAction(Node* node, Node* parent, int pos) : m_pos(pos)
{
    m_old_selected = wxGetFrame().getSelectedNodePtr();
    m_node = node->get_SharedPtr();
    m_parent = parent->get_SharedPtr();
    m_old_pos = m_parent->get_ChildPosition(node);

    m_undo_string << "Append " << map_GenNames[node->get_GenName()];

    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
    m_UndoEventGenerated = true;
    m_UndoSelectEventGenerated = true;
}

void AppendGridBagAction::Change()
{
    m_node->set_Parent(m_parent);
    if (m_pos == -1 && m_parent->get_ChildCount() > 0 &&
        m_parent->get_ChildPtr(m_parent->get_ChildCount() - 1)->is_Gen(gen_wxStdDialogButtonSizer))
    {
        m_parent->AddChild(m_node);
        m_parent->ChangeChildPosition(m_node, m_parent->get_ChildCount() - 2);
    }
    else
    {
        m_parent->AddChild(m_node);
        if (m_pos >= 0)
            m_parent->ChangeChildPosition(m_node, m_pos);
    }

    if (m_fix_duplicate_names)
    {
        // This needs to be done only once, even if the insertion is reverted and then changed
        // again. The reason is that any name changes to other nodes cannot be undone.

        m_node->FixDuplicateNodeNames();
        m_fix_duplicate_names = false;
    }

    wxGetFrame().FireCreatedEvent(m_node);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_node, evt_flags::fire_event | evt_flags::force_selection);
}

void AppendGridBagAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());

    wxGetFrame().FireDeletedEvent(m_node.get());
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_old_selected.get());
}

///////////////////////////////// GridBagAction ////////////////////////////////////

GridBagAction::GridBagAction(Node* cur_gbsizer, const tt_string& undo_str) :
    UndoAction(undo_str.c_str())
{
    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
    m_UndoEventGenerated = true;
    m_UndoSelectEventGenerated = true;

    m_cur_gbsizer = cur_gbsizer->get_SharedPtr();
    m_old_gbsizer = NodeCreation.MakeCopy(cur_gbsizer);

    auto nav_panel = wxGetFrame().getNavigationPanel();

    // Thaw() is called when GridBagAction::Update() is called
    nav_panel->Freeze();

    for (const auto& child: cur_gbsizer->get_ChildNodePtrs())
    {
        nav_panel->EraseAllMaps(child.get());
    }
}

void GridBagAction::Change()
{
    if (m_isReverted)
    {
        auto nav_panel = wxGetFrame().getNavigationPanel();
        wxWindowUpdateLocker freeze(nav_panel);

        for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
        {
            nav_panel->EraseAllMaps(child.get());
        }

        auto save = NodeCreation.MakeCopy(m_cur_gbsizer);
        m_cur_gbsizer->removeAllChildren();

        for (const auto& child: m_old_gbsizer->get_ChildNodePtrs())
        {
            m_cur_gbsizer->AdoptChild(NodeCreation.MakeCopy(child.get()));
        }
        m_old_gbsizer = std::move(save);
        m_isReverted = false;

        nav_panel->AddAllChildren(m_cur_gbsizer.get());
        nav_panel->ExpandAllNodes(m_cur_gbsizer.get());

        wxGetFrame().FireGridBagActionEvent(this);
        if (isAllowedSelectEvent())
            wxGetFrame().SelectNode(m_cur_gbsizer);
    }
}

void GridBagAction::Revert()
{
    auto nav_panel = wxGetFrame().getNavigationPanel();
    wxWindowUpdateLocker freeze(nav_panel);

    for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
    {
        nav_panel->EraseAllMaps(child.get());
    }

    auto save = NodeCreation.MakeCopy(m_cur_gbsizer);
    m_cur_gbsizer->removeAllChildren();
    for (const auto& child: m_old_gbsizer->get_ChildNodePtrs())
    {
        m_cur_gbsizer->AdoptChild(NodeCreation.MakeCopy(child.get()));
    }
    m_old_gbsizer = std::move(save);
    m_isReverted = true;

    nav_panel->AddAllChildren(m_cur_gbsizer.get());
    nav_panel->ExpandAllNodes(m_cur_gbsizer.get());

    wxGetFrame().FireGridBagActionEvent(this);
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(m_cur_gbsizer);
}

void GridBagAction::Update()
{
    auto nav_panel = wxGetFrame().getNavigationPanel();

    for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
    {
        nav_panel->EraseAllMaps(child.get());
    }

    nav_panel->AddAllChildren(m_cur_gbsizer.get());
    nav_panel->ExpandAllNodes(m_cur_gbsizer.get());
    nav_panel->Thaw();
}

///////////////////////////////// SortProjectAction ////////////////////////////////////

static bool CompareClassNames(NodeSharedPtr a, NodeSharedPtr b)
{
    // Sort folders first, then forms
    if (a->is_Gen(gen_folder) && !b->is_Gen(gen_folder))
        return true;
    else if (a->is_Gen(gen_folder) && b->is_Gen(gen_folder))
        return (a->as_string(prop_label).compare(b->as_string(prop_label)) < 0);
    else if (a->is_Gen(gen_sub_folder) && !b->is_Gen(gen_sub_folder))
        return true;
    else if (a->is_Gen(gen_sub_folder) && b->is_Gen(gen_sub_folder))
        return (a->as_string(prop_label).compare(b->as_string(prop_label)) < 0);
    return (a->as_string(prop_class_name).compare(b->as_string(prop_class_name)) < 0);
}

SortProjectAction::SortProjectAction()
{
    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
    m_UndoEventGenerated = true;
    m_UndoSelectEventGenerated = true;

    m_undo_string = "Sort Project";

    m_old_project = NodeCreation.MakeCopy(Project.get_ProjectNode());
}

void SortProjectAction::Change()
{
    auto& children = Project.get_ChildNodePtrs();
    std::sort(children.begin(), children.end(), CompareClassNames);

    for (auto& iter: Project.get_ChildNodePtrs())
    {
        if (iter->is_Gen(gen_folder) || iter->is_Gen(gen_sub_folder))
        {
            SortFolder(iter.get());
        }
    }

    wxGetFrame().FireProjectUpdatedEvent();
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(Project.get_ProjectNode());
}

void SortProjectAction::SortFolder(Node* folder)
{
    auto& children = folder->get_ChildNodePtrs();
    std::sort(children.begin(), children.end(), CompareClassNames);

    for (auto& iter: folder->get_ChildNodePtrs())
    {
        if (iter->is_Gen(gen_sub_folder))
        {
            SortFolder(iter.get());
        }
    }
}

void SortProjectAction::Revert()
{
    Project.get_ProjectNode()->removeAllChildren();
    for (const auto& child: m_old_project->get_ChildNodePtrs())
    {
        Project.get_ProjectNode()->AdoptChild(NodeCreation.MakeCopy(child.get()));
    }

    wxGetFrame().FireProjectUpdatedEvent();
    if (isAllowedSelectEvent())
        wxGetFrame().SelectNode(Project.get_ProjectNode());
}
