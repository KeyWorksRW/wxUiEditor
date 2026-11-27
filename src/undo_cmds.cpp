/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undoable command classes derived from UndoAction
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-21-2026]

#include <array>
#include <tuple>
#include <wx/filename.h>  // wxFileName - encapsulates a file path
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

// When an image node is added to or removed from an Images List, the form used by code
// generation for that image must be recalculated. See issue #1534.
static void UpdateImageFormForNode(Node* node)
{
    if (node->is_Gen(gen_embedded_image))
    {
        ProjectImages.UpdateImageForm(node->as_string(prop_bitmap));
    }
}

static bool ContainsImagesForm(Node* node)
{
    if (node->is_Gen(gen_Images))
    {
        return true;
    }

    // The Images form can live inside a project-level folder.
    if (node->is_Gen(gen_folder) || node->is_Gen(gen_sub_folder))
    {
        for (const auto& child: node->get_ChildNodePtrs())
        {
            if (ContainsImagesForm(child.get()))
            {
                return true;
            }
        }
    }

    return false;
}

// Removing or restoring an Images form invalidates ImageHandler's cached bundles and
// embedded images, so it has to be rebuilt from the current node tree. See issue #1291.
static void ReinitializeImagesForNode(Node* node)
{
    if (ContainsImagesForm(node))
    {
        Project.ReinitializeImageHandler();
    }
}

///////////////////////////////// InsertNodeAction ////////////////////////////////////

InsertNodeAction::InsertNodeAction(Node* node, Node* parent, std::string_view undo_str, int pos) :
    m_old_selected(wxGetFrame().getSelectedNodePtr()),
    m_pos(pos)
{
    m_node = node->get_SharedPtr();
    m_parent = parent->get_SharedPtr();
    SetUndoString(undo_str);

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
        {
            m_parent->ChangeChildPosition(m_node, m_pos);
        }
    }

    UpdateImageFormForNode(m_node.get());
    ReinitializeImagesForNode(m_node.get());

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
    {
        wxGetFrame().SelectNode(m_node.get(), evt_flags::no_event);
    }
}

void InsertNodeAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());  // Remove the parent pointer

    UpdateImageFormForNode(m_node.get());
    ReinitializeImagesForNode(m_node.get());

    // Revert is only reached after Change() inserted the node, so the deleted event has to fire
    // even when the caller pre-fired the created event and m_fire_created_event is false.
    wxGetFrame().FireDeletedEvent(m_node.get());

    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_old_selected.get());
    }
}

bool InsertNodeAction::IsReadOnly() const
{
    return m_parent && m_parent->is_ReadOnly();
}

///////////////////////////////// RemoveNodeAction ////////////////////////////////////

RemoveNodeAction::RemoveNodeAction(Node* node, std::string_view undo_str, bool AddToClipboard) :
    m_old_selected(wxGetFrame().getSelectedNodePtr()),
    m_AddToClipboard(AddToClipboard)
{
    m_node = node->get_SharedPtr();
    m_parent = m_node->get_ParentPtr();
    m_old_pos = m_parent->get_ChildPosition(m_node);

    SetUndoString(undo_str);

    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
}

void RemoveNodeAction::Change()
{
    if (m_AddToClipboard)
    {
        m_node->CopyNode();
    }

    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());

    UpdateImageFormForNode(m_node.get());
    ReinitializeImagesForNode(m_node.get());

    wxGetFrame().FireDeletedEvent(m_node.get());

    if (m_parent->get_ChildCount())
    {
        const size_t pos =
            (m_old_pos < m_parent->get_ChildCount() ? m_old_pos : m_parent->get_ChildCount() - 1);
        if (isAllowedSelectEvent())
        {
            wxGetFrame().SelectNode(m_parent->get_Child(pos));
        }
    }
    else
    {
        if (isAllowedSelectEvent())
        {
            wxGetFrame().SelectNode(m_parent.get());
        }
    }
}

void RemoveNodeAction::Revert()
{
    m_parent->AddChild(m_node);
    m_node->set_Parent(m_parent);
    m_parent->ChangeChildPosition(m_node, m_old_pos);

    UpdateImageFormForNode(m_node.get());
    ReinitializeImagesForNode(m_node.get());

    wxGetFrame().FireCreatedEvent(m_node.get());

    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_old_selected.get(), evt_flags::force_selection);
    }
}

bool RemoveNodeAction::IsReadOnly() const
{
    return (m_node && m_node->is_ReadOnly()) || (m_parent && m_parent->is_ReadOnly());
}

///////////////////////////////// ModifyPropertyAction ////////////////////////////////////

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, std::string_view value) :
    m_property(prop),
    m_revert_value(prop->as_string())
{
    m_undo_string << "change " << prop->get_DeclName();

    m_change_value << value;

    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

ModifyPropertyAction::ModifyPropertyAction(NodeProperty* prop, int value) :
    m_property(prop),
    m_revert_value(prop->as_string())
{
    m_undo_string << "change " << prop->get_DeclName();

    m_change_value << value;

    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

void ModifyPropertyAction::Change()
{
    m_property->set_value(m_change_value);

    if (m_property->isProp(prop_art_directory) || m_property->isProp(prop_xrc_art_directory))
    {
        // Every embedded image was resolved against the old art directory, so the cache has to be
        // rebuilt from the new location. See issue #1670.
        Project.ReinitializeImageHandler();
    }

    wxGetFrame().FirePropChangeEvent(m_property);
}

void ModifyPropertyAction::Revert()
{
    m_property->set_value(m_revert_value);

    if (m_property->isProp(prop_art_directory) || m_property->isProp(prop_xrc_art_directory))
    {
        // Every embedded image was resolved against the old art directory, so the cache has to be
        // rebuilt from the new location. See issue #1670.
        Project.ReinitializeImageHandler();
    }

    wxGetFrame().FirePropChangeEvent(m_property);
}

bool ModifyPropertyAction::IsReadOnly() const
{
    const Node* node = m_property ? m_property->getNode() : nullptr;
    return node && node->is_ReadOnly();
}

///////////////////////////////// ModifyProperties ////////////////////////////////////

ModifyProperties::ModifyProperties(std::string_view undo_string, bool fire_events) :
    UndoAction(undo_string),
    m_fire_events(fire_events)
{
    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

void ModifyProperties::addProperty(NodeProperty* prop, std::string_view value)
{
    if (prop->isProp(prop_art_directory) || prop->isProp(prop_xrc_art_directory))
    {
        m_images_invalidated = true;
    }

    MULTI_PROP& entry = m_properties.emplace_back();
    entry.property = prop;
    entry.change_value = value;
    entry.revert_value = prop->as_string();
}

void ModifyProperties::addProperty(NodeProperty* prop, int value)
{
    MULTI_PROP& entry = m_properties.emplace_back();
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

    if (m_images_invalidated)
    {
        // Embedded images were resolved against the old art directory. See issue #1670.
        Project.ReinitializeImageHandler();
    }

    if (m_fire_events)
    {
        wxGetFrame().FireMultiPropEvent(this);
    }
}

void ModifyProperties::Revert()
{
    // Undo in reverse insertion order: a later entry may have been computed from the value an
    // earlier entry reverts, so the last one recorded must be restored first.
    for (auto iter = m_properties.rbegin(); iter != m_properties.rend(); ++iter)
    {
        iter->property->set_value(iter->revert_value);
    }

    if (m_images_invalidated)
    {
        // Embedded images were resolved against the old art directory. See issue #1670.
        Project.ReinitializeImageHandler();
    }

    if (m_fire_events)
    {
        wxGetFrame().FireMultiPropEvent(this);
    }
}

size_t ModifyProperties::GetMemorySize()
{
    size_t total = sizeof(*this) + (m_properties.size() * sizeof(MULTI_PROP));
    for (auto& iter: m_properties)
    {
        // The +2 is to account for the trailing zero in each std::string value.
        total += (iter.change_value.size() + iter.revert_value.size() + 2);
    }
    return total;
}

bool ModifyProperties::IsReadOnly() const
{
    for (const auto& iter: m_properties)
    {
        const Node* node = iter.property ? iter.property->getNode() : nullptr;
        if (node && node->is_ReadOnly())
        {
            return true;
        }
    }
    return false;
}

///////////////////////////////// LinkProjectAction ////////////////////////////////////

LinkProjectAction::LinkProjectAction(Node* folder, std::string_view new_value) :
    m_folder(folder ? folder->get_SharedPtr() : NodeSharedPtr())
{
    m_undo_string = "change link_project";

    if (m_folder && m_folder->HasProp(prop_link_project))
    {
        m_revert_value = m_folder->as_string(prop_link_project);
    }

    // Store the value relative to the project path, the same way a file property is stored.
    // A relative value is resolved against the project directory (never the current working
    // directory) because that is how the loader resolves link_project.
    wxue::string normalized(new_value);
    if (!normalized.empty())
    {
        wxFileName file(normalized.wx());
        if (!file.IsAbsolute())
        {
            file.MakeAbsolute(Project.get_ProjectPath().wx());
        }
        file.MakeRelativeTo(Project.get_ProjectPath().wx());
        normalized = file.GetFullPath().ToStdString();
        normalized.backslashestoforward();
    }
    m_change_value = normalized;

    m_RedoEventGenerated = true;
    m_UndoEventGenerated = true;
}

void LinkProjectAction::Change()
{
    ApplyLink(m_change_value, m_revert_value, false);
}

void LinkProjectAction::Revert()
{
    ApplyLink(m_revert_value, m_change_value, true);
}

void LinkProjectAction::ApplyLink(const wxue::string& value, const wxue::string& fallback,
                                  bool replace_silently)
{
    if (!m_folder || !m_folder->HasProp(prop_link_project))
    {
        return;
    }

    NodeProperty* property = m_folder->get_PropPtr(prop_link_project);
    if (property->as_string() == value)
    {
        // Nothing to apply -- this is the case when the user refused to replace the existing
        // children and the property was already put back.
        return;
    }

    property->set_value(value);

    bool load_succeeded = true;
    if (value.empty())
    {
        Project.UnloadLinkedProject(m_folder.get());
    }
    else if (replace_silently)
    {
        // Undo/redo: the user already asked for this state, so replace the children without a
        // second confirmation.
        load_succeeded = Project.ReloadLinkedProject(m_folder.get(), Project.is_UiAllowed());
    }
    else
    {
        // LoadLinkedProject (not ReloadLinkedProject) replaces the children itself, and only
        // after the user confirms that the existing children may be discarded.
        load_succeeded = Project.LoadLinkedProject(m_folder.get(), Project.is_UiAllowed());
    }

    if (!load_succeeded)
    {
        // The load was refused or failed. Put the property back so the property grid matches the
        // tree, and silently re-read the previous link in case the failure happened after its
        // children had already been unloaded.
        property->set_value(fallback);
        if (fallback.empty())
        {
            // There is no previous link to restore, so make sure no partially loaded children are
            // left behind.
            Project.UnloadLinkedProject(m_folder.get());
        }
        else if (!Project.ReloadLinkedProject(m_folder.get(), false))
        {
            MSG_ERROR("Failed to restore the previous linked project");
        }
    }

    wxGetFrame().FirePropChangeEvent(property);
    if (NavigationPanel* nav_panel = wxGetFrame().getNavigationPanel(); nav_panel != nullptr)
    {
        nav_panel->RefreshParent(m_folder.get());
    }
}

///////////////////////////////// ModifyEventAction ////////////////////////////////////

ModifyEventAction::ModifyEventAction(NodeEvent* event, std::string_view value) :
    m_event(event),
    m_revert_value(event->get_value()),
    m_change_value(value)
{
    m_undo_string << "change " << event->get_name() << " handler";

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

bool ModifyEventAction::IsReadOnly() const
{
    const Node* node = m_event ? m_event->getNode() : nullptr;
    return node && node->is_ReadOnly();
}

///////////////////////////////// ChangePositionAction ////////////////////////////////////

ChangePositionAction::ChangePositionAction(Node* node, size_t position) : m_change_pos(position)
{
    m_node = node->get_SharedPtr();
    m_parent = m_node->get_ParentPtr();

    m_revert_pos = m_parent->get_ChildPosition(m_node);

    SetUndoString(wxue::string() << "change " << m_node->get_DeclName() << " position");

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
    {
        wxGetFrame().SelectNode(m_node);
    }
}

void ChangePositionAction::Revert()
{
    m_parent->ChangeChildPosition(m_node, m_revert_pos);
    wxGetFrame().FirePositionChangedEvent(this);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_node);
    }
}

bool ChangePositionAction::IsReadOnly() const
{
    return (m_node && m_node->is_ReadOnly()) || (m_parent && m_parent->is_ReadOnly());
}

// Defined with the ChangeNodeType helpers below; declared here because ChangeSizerType uses it.
static void CopyCommonProperties(Node* old_node, Node* new_node);

/////////////////////////////// ChangeSizerType ////////////////////////////////////

ChangeSizerType::ChangeSizerType(Node* node, GenEnum::GenName new_sizer) :
    m_new_gen_sizer(new_sizer)
{
    m_undo_string << "change sizer type";

    m_old_node = node->get_SharedPtr();
    m_parent = node->get_ParentPtr();

    m_node = NodeCreation.NewNode(m_new_gen_sizer);
    ASSERT(m_node);
    if (m_node)
    {
        CopyCommonProperties(m_old_node.get(), m_node.get());
        const std::string new_name =
            m_old_node->get_UniqueName(m_node->as_string(prop_var_name), prop_var_name);
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

        // These children are deep copies, so undoing the conversion restores the originals. Any
        // action captured before the conversion still holds NodeSharedPtrs to those originals,
        // therefore the conversion must be undone first (LIFO undo order) and no captured pointer
        // may be used while these copies are live.
        for (const auto& iter: m_old_node->get_ChildNodePtrs())
        {
            m_node->AdoptChild(NodeCreation.MakeCopy(iter.get()));
        }
    }
}

void ChangeSizerType::Change()
{
    if (!m_node)
    {
        return;
    }

    const size_t pos = m_parent->get_ChildPosition(m_old_node.get());
    m_parent->RemoveChild(m_old_node);
    m_old_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_node);
    m_parent->ChangeChildPosition(m_node, pos);

    wxGetFrame().FireDeletedEvent(m_old_node.get());
    wxGetFrame().FireCreatedEvent(m_node);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_node.get());
    }
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_node.get());
}

void ChangeSizerType::Revert()
{
    if (!m_node)
    {
        return;
    }

    const size_t pos = m_parent->get_ChildPosition(m_node.get());
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_old_node);
    m_parent->ChangeChildPosition(m_old_node, pos);

    wxGetFrame().FireDeletedEvent(m_node.get());
    wxGetFrame().FireCreatedEvent(m_old_node);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_old_node.get());
    }
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_old_node.get());
}

bool ChangeSizerType::IsReadOnly() const
{
    return (m_old_node && m_old_node->is_ReadOnly()) || (m_parent && m_parent->is_ReadOnly());
}

///////////////////////////////// ChangeNodeType ////////////////////////////////////

static constexpr std::array lst_common_properties = {

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
    prop_conditional,
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
            std::ignore = new_node->set_value(prop, old_node->as_string(prop));
        }
    }

    if (old_node->HasProp(prop_var_name) && old_node->as_string(prop_class_access) != "none")
    {
        std::ignore = new_node->set_value(prop_var_name, old_node->as_string(prop_var_name));
    }

    if (old_node->HasValue(prop_validator_variable) && new_node->HasProp(prop_validator_variable))
    {
        std::ignore = new_node->set_value(prop_validator_variable,
                                          old_node->as_string(prop_validator_variable));
        if (new_node->HasProp(prop_validator_data_type))
        {
            std::ignore = new_node->set_value(prop_validator_data_type,
                                              old_node->as_string(prop_validator_data_type));
        }
    }

    if (old_node->is_Gen(gen_wxComboBox) && new_node->is_Gen(gen_wxChoice))
    {
        NodeMapEvents map_old_events = old_node->get_MapEvents();
        if (const NodeMapEvents::iterator event = map_old_events.find("wxEVT_COMBOBOX");
            event != map_old_events.end())
        {
            if (!event->second.get_value().empty())
            {
                NodeEvent* new_event = new_node->get_Event("wxEVT_CHOICE");
                if (new_event)
                {
                    new_event->set_value(event->second.get_value());
                }
            }
        }
    }
    else if (old_node->is_Gen(gen_wxChoice) && new_node->is_Gen(gen_wxComboBox))
    {
        // Inverse of the remap above, so a Choice converted to a ComboBox keeps its handler.
        NodeMapEvents map_old_events = old_node->get_MapEvents();
        if (const NodeMapEvents::iterator event = map_old_events.find("wxEVT_CHOICE");
            event != map_old_events.end())
        {
            if (!event->second.get_value().empty())
            {
                NodeEvent* new_event = new_node->get_Event("wxEVT_COMBOBOX");
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
        const std::string new_name =
            m_old_node->get_UniqueName(m_node->as_string(prop_var_name), prop_var_name);
        CopyCommonProperties(m_old_node.get(), m_node.get());
        m_node->set_value(prop_var_name, new_name);
        if ((m_new_gen_node == gen_wxCheckBox || m_new_gen_node == gen_wxRadioBox) &&
            m_old_node->HasProp(prop_checked))
        {
            m_node->set_value(prop_checked, m_old_node->as_bool(prop_checked));
        }

        // These children are deep copies, so undoing the conversion restores the originals. Any
        // action captured before the conversion still holds NodeSharedPtrs to those originals,
        // therefore the conversion must be undone first (LIFO undo order) and no captured pointer
        // may be used while these copies are live.
        for (const auto& iter: m_old_node->get_ChildNodePtrs())
        {
            m_node->AdoptChild(NodeCreation.MakeCopy(iter.get()));
        }
    }
}

void ChangeNodeType::Change()
{
    if (!m_node)
    {
        return;
    }

    const size_t pos = m_parent->get_ChildPosition(m_old_node.get());
    m_parent->RemoveChild(m_old_node);
    m_old_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_node);
    m_parent->ChangeChildPosition(m_node, pos);

    wxGetFrame().FireDeletedEvent(m_old_node.get());
    wxGetFrame().FireCreatedEvent(m_node);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_node.get());
    }
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_node.get());
}

void ChangeNodeType::Revert()
{
    if (!m_node)
    {
        return;
    }

    const size_t pos = m_parent->get_ChildPosition(m_node.get());
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());
    m_parent->AdoptChild(m_old_node);
    m_parent->ChangeChildPosition(m_old_node, pos);

    wxGetFrame().FireDeletedEvent(m_node.get());
    wxGetFrame().FireCreatedEvent(m_old_node);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_old_node.get());
    }
    wxGetFrame().getNavigationPanel()->ChangeExpansion(m_old_node.get());
}

bool ChangeNodeType::IsReadOnly() const
{
    return (m_old_node && m_old_node->is_ReadOnly()) || (m_parent && m_parent->is_ReadOnly());
}

///////////////////////////////// ChangeParentAction ////////////////////////////////////

ChangeParentAction::ChangeParentAction(Node* node, Node* parent, int pos)
{
    m_node = node->get_SharedPtr();
    m_change_parent = parent->get_SharedPtr();
    m_revert_parent = m_node->get_ParentPtr();
    m_pos = pos;

    m_revert_position = m_revert_parent->get_ChildPosition(m_node);
    if (const NodeProperty* row_prop = m_node->get_PropPtr(prop_row); row_prop)
    {
        m_revert_row = row_prop->as_int();
    }
    if (const NodeProperty* col_prop = m_node->get_PropPtr(prop_column); col_prop)
    {
        m_revert_col = col_prop->as_int();
    }

    SetUndoString(wxue::string() << "change " << m_node->get_DeclName() << " parent");

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
            {
                wxGetFrame().SelectNode(m_node);
            }
        }
        else
        {
            m_node->set_Parent(m_change_parent);
            wxGetFrame().FireParentChangedEvent(this);
            if (isAllowedSelectEvent())
            {
                wxGetFrame().SelectNode(m_node);
            }
        }
    }
    else
    {
        bool result = false;
        if (m_pos >= 0)
        {
            result = m_change_parent->AddChild(static_cast<size_t>(m_pos), m_node);
        }
        else
        {
            result = m_change_parent->AddChild(m_node);
        }
        ASSERT_MSG(result, wxue::string("Unable to change parent of ")
                               << m_node->get_NodeName() << " to "
                               << m_change_parent->get_NodeName());
        if (result)
        {
            m_node->set_Parent(m_change_parent);

            wxGetFrame().FireParentChangedEvent(this);
            if (isAllowedSelectEvent())
            {
                wxGetFrame().SelectNode(m_node);
            }
        }
        else
        {
            m_node->set_Parent(m_revert_parent);
            m_revert_parent->AddChild(m_node);
            m_revert_parent->ChangeChildPosition(m_node, m_revert_position);
            // Since we deleted it from Navigation Panel, need to add it back
            wxGetFrame().FireParentChangedEvent(this);
            if (isAllowedSelectEvent())
            {
                wxGetFrame().SelectNode(m_node);
            }
        }
    }
}

void ChangeParentAction::Revert()
{
    m_change_parent->RemoveChild(m_node);

    m_node->set_Parent(m_revert_parent);
    m_revert_parent->AddChild(m_node);
    m_revert_parent->ChangeChildPosition(m_node, m_revert_position);
    if (NodeProperty* prop = m_node->get_PropPtr(prop_row); prop)
    {
        prop->set_value(m_revert_row);
    }
    if (NodeProperty* prop = m_node->get_PropPtr(prop_column); prop)
    {
        prop->set_value(m_revert_col);
    }

    wxGetFrame().FireParentChangedEvent(this);
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_node);
    }
}

bool ChangeParentAction::IsReadOnly() const
{
    return (m_node && m_node->is_ReadOnly()) ||
           (m_change_parent && m_change_parent->is_ReadOnly()) ||
           (m_revert_parent && m_revert_parent->is_ReadOnly());
}

///////////////////////////////// AppendGridBagAction ////////////////////////////////////

AppendGridBagAction::AppendGridBagAction(Node* node, Node* parent, int pos) :
    m_old_selected(wxGetFrame().getSelectedNodePtr()),
    m_pos(pos)
{
    m_node = node->get_SharedPtr();
    m_parent = parent->get_SharedPtr();

    ASSERT(map_GenNames.contains(node->get_GenName()));

    m_undo_string << "Append " << std::string(map_GenNames.at(node->get_GenName()));

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
        {
            m_parent->ChangeChildPosition(m_node, m_pos);
        }
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
    {
        wxGetFrame().SelectNode(m_node, evt_flags::fire_event | evt_flags::force_selection);
    }
}

void AppendGridBagAction::Revert()
{
    m_parent->RemoveChild(m_node);
    m_node->set_Parent(NodeSharedPtr());

    wxGetFrame().FireDeletedEvent(m_node.get());
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(m_old_selected.get());
    }
}

bool AppendGridBagAction::IsReadOnly() const
{
    return m_parent && m_parent->is_ReadOnly();
}

///////////////////////////////// GridBagAction ////////////////////////////////////

GridBagAction::GridBagAction(Node* cur_gbsizer, std::string_view undo_str) : UndoAction(undo_str)
{
    m_RedoEventGenerated = true;
    m_RedoSelectEventGenerated = true;
    m_UndoEventGenerated = true;
    m_UndoSelectEventGenerated = true;

    m_cur_gbsizer = cur_gbsizer->get_SharedPtr();
    m_old_gbsizer = NodeCreation.MakeCopy(cur_gbsizer);

    NavigationPanel* nav_panel = wxGetFrame().getNavigationPanel();

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
        NavigationPanel* nav_panel = wxGetFrame().getNavigationPanel();
        const wxWindowUpdateLocker freeze(nav_panel);

        for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
        {
            nav_panel->EraseAllMaps(child.get());
        }

        NodeSharedPtr save = NodeCreation.MakeCopy(m_cur_gbsizer);
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
        {
            wxGetFrame().SelectNode(m_cur_gbsizer);
        }
    }
}

void GridBagAction::Revert()
{
    NavigationPanel* nav_panel = wxGetFrame().getNavigationPanel();
    const wxWindowUpdateLocker freeze(nav_panel);

    for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
    {
        nav_panel->EraseAllMaps(child.get());
    }

    NodeSharedPtr save = NodeCreation.MakeCopy(m_cur_gbsizer);
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
    {
        wxGetFrame().SelectNode(m_cur_gbsizer);
    }
}

void GridBagAction::Update()
{
    NavigationPanel* nav_panel = wxGetFrame().getNavigationPanel();

    for (const auto& child: m_cur_gbsizer->get_ChildNodePtrs())
    {
        nav_panel->EraseAllMaps(child.get());
    }

    nav_panel->AddAllChildren(m_cur_gbsizer.get());
    nav_panel->ExpandAllNodes(m_cur_gbsizer.get());
    nav_panel->Thaw();
}

bool GridBagAction::IsReadOnly() const
{
    return m_cur_gbsizer && m_cur_gbsizer->is_ReadOnly();
}

///////////////////////////////// SortProjectAction ////////////////////////////////////

static bool CompareClassNames(NodeSharedPtr node_a, NodeSharedPtr node_b)
{
    // Sort folders first, then forms
    if (node_a->is_Gen(gen_folder) && !node_b->is_Gen(gen_folder))
    {
        return true;
    }
    if (!node_a->is_Gen(gen_folder) && node_b->is_Gen(gen_folder))
    {
        return false;
    }
    if (node_a->is_Gen(gen_folder) && node_b->is_Gen(gen_folder))
    {
        return (node_a->as_string(prop_label).compare(node_b->as_string(prop_label)) < 0);
    }
    if (node_a->is_Gen(gen_sub_folder) && !node_b->is_Gen(gen_sub_folder))
    {
        return true;
    }
    if (node_a->is_Gen(gen_sub_folder) && node_b->is_Gen(gen_sub_folder))
    {
        return (node_a->as_string(prop_label).compare(node_b->as_string(prop_label)) < 0);
    }
    return (node_a->as_string(prop_class_name).compare(node_b->as_string(prop_class_name)) < 0);
}

// True when the pre-sort copy describes the same node as the live one. The properties compared
// here are the ones CompareClassNames sorts on, plus the variable name for tie-breaks.
static bool IsSameSortEntry(const Node& old_node, const Node& live_node)
{
    return old_node.as_string(prop_class_name) == live_node.as_string(prop_class_name) &&
           old_node.as_string(prop_label) == live_node.as_string(prop_label) &&
           old_node.as_string(prop_var_name) == live_node.as_string(prop_var_name);
}

// Reorders the live children to match the order captured in old_folder (a copy taken before the
// sort). The children themselves are never replaced, because earlier undo actions, the Navigation
// Panel and the current selection still hold NodeSharedPtrs to the live nodes.
static void RestoreChildOrder(Node* live_folder, Node* old_folder)
{
    std::vector<NodeSharedPtr>& live_children = live_folder->get_ChildNodePtrs();
    const std::vector<NodeSharedPtr>& old_children = old_folder->get_ChildNodePtrs();
    if (old_children.size() != live_children.size())
    {
        // The tree changed after the sort, so there is no order left to restore.
        return;
    }

    std::vector<NodeSharedPtr> restored_children;
    restored_children.reserve(live_children.size());
    std::vector<bool> claimed(live_children.size(), false);
    for (const auto& old_child: old_children)
    {
        for (size_t child_index = 0; child_index < live_children.size(); ++child_index)
        {
            if (!claimed[child_index] && IsSameSortEntry(*old_child, *live_children[child_index]))
            {
                claimed[child_index] = true;
                restored_children.push_back(live_children[child_index]);
                if (!live_children[child_index]->get_ChildNodePtrs().empty())
                {
                    RestoreChildOrder(live_children[child_index].get(), old_child.get());
                }
                break;
            }
        }
    }

    if (restored_children.size() == live_children.size())
    {
        live_children = std::move(restored_children);
    }
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
    std::vector<NodeSharedPtr>& children = Project.get_ChildNodePtrs();
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
    {
        wxGetFrame().SelectNode(Project.get_ProjectNode());
    }
}

void SortProjectAction::SortFolder(Node* folder)
{
    std::vector<NodeSharedPtr>& children = folder->get_ChildNodePtrs();
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
    // Restore the order in place: replacing the children with copies would detach the
    // NodeSharedPtrs still held by earlier undo actions, the Navigation Panel and the selection.
    RestoreChildOrder(Project.get_ProjectNode(), m_old_project.get());

    wxGetFrame().FireProjectUpdatedEvent();
    if (isAllowedSelectEvent())
    {
        wxGetFrame().SelectNode(Project.get_ProjectNode());
    }
}
