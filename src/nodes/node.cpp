/////////////////////////////////////////////////////////////////////////////
// Purpose:   Contains user-modifiable node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/animate.h>  // wxAnimation and wxAnimationCtrl
#include <wx/sizer.h>    // provide wxSizer class for layout

#include <frozen/set.h>

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

#include "wxue_namespace/wxue.h"         // wxue helpers for character classification
#include "wxue_namespace/wxue_string.h"  // wxue::string class

using namespace GenEnum;
using enum Node::Validity;

// clang-format off
inline constexpr auto lst_form_types = std::to_array<GenType>({

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

});

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

auto Node::is_Form() const noexcept -> bool
{
    return std::ranges::any_of(lst_form_types,
                               [this](GenType type)
                               {
                                   return is_Type(type);
                               });
}

Node::Node(NodeDeclaration* declaration) : m_declaration(declaration) {}

auto Node::get_PropPtr(PropName name) -> NodeProperty*
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return &m_properties[result->second];
    }
    return nullptr;
}

auto Node::get_Event(std::string_view name) -> NodeEvent*
{
    if (auto iter = m_map_events.find(name); iter != m_map_events.end())
    {
        return &iter->second;
    }

    return nullptr;
}

auto Node::get_InUseEventCount() const -> size_t
{
    size_t count = 0;

    for (const auto& iter: m_map_events)
    {
        if (iter.second.get_value().size())
        {
            ++count;
        }
    }

    return count;
}

auto Node::AddNodeProperty(PropDeclaration* declaration) -> NodeProperty*
{
    auto& prop = m_properties.emplace_back(declaration, this);
    m_prop_indices[prop.get_name()] = (m_properties.size() - 1);
    return &m_properties[m_properties.size() - 1];
}

auto Node::AddNodeEvent(const NodeEventInfo* info) -> void
{
    m_map_events.emplace(info->get_name(), NodeEvent(info, this));
}

auto Node::CopyEventsFrom(Node* from) -> void
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

auto Node::get_Form() noexcept -> Node*
{
    if (is_Form())
    {
        return this;
    }

    for (auto* parent = get_Parent(); parent; parent = parent->get_Parent())
    {
        if (parent->is_Form())
        {
            return parent;
        }
    }

    return nullptr;
}

auto Node::get_Folder() noexcept -> Node*  // NOLINT (cppcheck-suppress)
{
    for (auto* parent = get_Parent(); parent; parent = parent->get_Parent())
    {
        if (parent->is_Gen(gen_folder) || parent->is_Gen(gen_data_folder))
        {
            return parent;
        }
    }

    return nullptr;
}

auto Node::get_ValidFormParent() noexcept -> Node*
{
    auto* parent = this;
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

auto Node::AdoptChild(const NodeSharedPtr& child) -> bool
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

auto Node::AddChild(const NodeSharedPtr& node) -> bool
{
    if (is_ChildAllowed(node))
    {
        m_children.push_back(node);
        return true;
    }

    return false;
}

auto Node::AddChild(Node* node) -> bool
{
    if (is_ChildAllowed(node))
    {
        m_children.push_back(node->get_SharedPtr());
        return true;
    }

    return false;
}

auto Node::AddChild(size_t idx, const NodeSharedPtr& node) -> bool
{
    if (is_ChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + static_cast<ptrdiff_t>(idx), node);
        return true;
    }

    return false;
}

auto Node::AddChild(size_t idx, Node* node) -> bool
{
    if (is_ChildAllowed(node) && idx <= m_children.size())
    {
        m_children.insert(m_children.begin() + static_cast<ptrdiff_t>(idx), node->get_SharedPtr());
        return true;
    }

    return false;
}

auto Node::is_ChildAllowed(NodeDeclaration* child) -> bool
{
    ASSERT(child);

    auto max_children = m_declaration->get_AllowableChildren(child->get_GenType());

    if (max_children == child_count::none)
    {
        return false;
    }

    if (max_children == child_count::infinite)
    {
        return true;
    }

    if (is_Gen(gen_wxSplitterWindow))
    {
        return (get_ChildCount() < 2);
    }

    // Because m_children contains shared_ptrs, we don't want to use an iteration loop which will
    // get/release the shared ptr. Using an index into the vector lets us access the raw pointer.

    ptrdiff_t children = 0;
    for (size_t i = 0; i < m_children.size() && children <= max_children; ++i)
    {
        auto child_type = get_Child(i)->get_GenType();
        // treat type-sizer and type_gbsizer as the same since forms and contains can only have
        // one of them as the top level sizer.
        if (child_type == child->get_GenType() ||
            (child->get_GenType() == type_sizer && child_type == type_gbsizer) ||
            (child->get_GenType() == type_gbsizer && child_type == type_sizer))
        {
            ++children;
        }
    }

    return children < max_children;
}

auto Node::is_ChildAllowed(Node* child) -> bool
{
    ASSERT(child);

    return is_ChildAllowed(child->get_NodeDeclaration());
}

auto Node::RemoveChild(Node* node) -> void
{
    for (size_t pos = 0; const auto& child: m_children)
    {
        if (child.get() == node)
        {
            m_children.erase(m_children.begin() + static_cast<ptrdiff_t>(pos));
            break;
        }
        ++pos;
        ASSERT_MSG(pos < m_children.size(), "failed to find child node!");
    }
}

auto Node::RemoveChild(size_t index) -> void
{
    ASSERT(index < m_children.size());

    auto iter = m_children.begin() + static_cast<ptrdiff_t>(index);
    m_children.erase(iter);
}

auto Node::get_ChildPosition(Node* node) -> size_t
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

auto Node::ChangeChildPosition(const NodeSharedPtr& node, size_t pos) -> bool
{
    size_t cur_pos = get_ChildPosition(node.get());

    if (cur_pos == get_ChildCount() || pos >= get_ChildCount())
    {
        return false;
    }

    if (pos == cur_pos)
    {
        return true;
    }

    RemoveChild(node);
    AddChild(pos, node);
    return true;
}

auto Node::is_Local() const noexcept -> bool
{
    return is_PropValue(prop_class_access, "none");
}

auto Node::HasValue(PropName name) const -> bool
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].HasValue();
    }
    return false;
}

auto Node::is_PropValue(PropName name, const char* value) const noexcept -> bool
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_string().is_sameas(value));
    }

    return false;
}

auto Node::is_PropValue(PropName name, bool value) const noexcept -> bool
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_bool() == value);
    }

    return false;
}

auto Node::is_PropValue(PropName name, int value) const noexcept -> bool
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return (m_properties[result->second].as_int() == value);
    }

    return false;
}

auto Node::as_mockup(PropName name, std::string_view prefix) const -> int
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_mockup(prefix);
    }
    return 0;
}

auto Node::as_ArrayString(PropName name) const -> std::vector<wxue::string>
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_ArrayString();
    }
    return std::vector<wxue::string>();
}

auto Node::get_PropValuePtr(PropName name) -> wxue::string*
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_raw_ptr();
    }
    return nullptr;
}

auto Node::get_PropId() const -> wxue::string
{
    wxue::string id_prop;
    if (auto result = m_prop_indices.find(prop_id); result != m_prop_indices.end())
    {
        id_prop = m_properties[result->second].get_PropId();
    }
    return id_prop;
}

auto Node::as_statusbar_fields(PropName name) -> std::vector<NODEPROP_STATUSBAR_FIELD>
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_statusbar_fields();
    }
    return {};
}

auto Node::as_checklist_items(PropName name) -> std::vector<NODEPROP_CHECKLIST_ITEM>
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_checklist_items();
    }
    return {};
}

auto Node::as_radiobox_items(PropName name) -> std::vector<NODEPROP_RADIOBOX_ITEM>
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_radiobox_items();
    }
    return {};
}

auto Node::as_bmp_combo_items(PropName name) -> std::vector<NODEPROP_BMP_COMBO_ITEM>
{
    if (auto result = m_prop_indices.find(name); result != m_prop_indices.end())
    {
        return m_properties[result->second].as_bmp_combo_items();
    }
    return {};
}

auto Node::get_PropDefaultValue(PropName name) -> std::string_view
{
    auto* prop = get_PropPtr(name);

    ASSERT_MSG(prop, wxue::string(get_NodeName())
                         << " doesn't have the property " << map_PropNames.at(name));

    if (prop)
    {
        return prop->getDefaultValue();
    }
    return {};
}

auto Node::get_NodeName() const -> std::string_view
{
    auto iter = m_prop_indices.find(prop_var_name);
    if (iter != m_prop_indices.end())
    {
        return m_properties[iter->second].as_view();
    }
    if (iter = m_prop_indices.find(prop_class_name); iter != m_prop_indices.end())
    {
        return m_properties[iter->second].as_view();
    }
    return {};
}

auto Node::get_NodeName(GenLang lang) const -> std::string_view
{
    std::string_view name = get_NodeName();
    if (name.empty())
    {
        return "unknown node";
    }
    if (lang == GEN_LANG_CPLUSPLUS)
    {
        // '@' is used for Ruby instance variables
        // '$' is commonly used for Perl package variables
        // '_' is used for local Python variables
        // In C++, these prefixes are not standard practice, so we remove them
        if (name[0] == '@' || name[0] == '$' || (name[0] == '_' && is_Local()))
        {
            name.remove_prefix(1);
        }
        return name;
    }

    if (name[0] == '@')
    {
        if (lang != GEN_LANG_RUBY)
        {
            name.remove_prefix(1);
        }
        return name;
    }
    if (name[0] == '$' && lang != GEN_LANG_PERL)
    {
        name.remove_prefix(1);
        return name;
    }

    // GEN_LANG_CPLUSPLUS is handled above
    ASSERT(lang != GEN_LANG_CPLUSPLUS);
    if (name.starts_with("m_"))
    {
        name.remove_prefix(2);
    }
    return name;
}

auto Node::get_ParentName() const -> std::string_view
{
    if (m_parent)
    {
        return m_parent->get_NodeName();
    }

    return std::string_view("");
}

auto Node::get_ParentName(GenLang lang, bool ignore_sizers) const -> std::string_view
{
    if (ignore_sizers)
    {
        auto* parent = get_Parent();
        while (parent && parent->is_Sizer())
        {
            parent = parent->get_Parent();
        }
        if (parent)
        {
            return parent->get_NodeName(lang);
        }
    }
    else
    {
        if (m_parent)
        {
            return m_parent->get_NodeName(lang);
        }
    }
    return wxue::wxue_empty_string;
}

auto Node::get_FormName() -> std::string_view
{
    if (auto* form = get_Form(); form)
    {
        return form->get_NodeName();
    }
    return std::string_view("");
}

auto Node::GetBorderDirection(std::string_view border_settings) -> int
{
    if (border_settings.contains("wxALL"))
    {
        return wxALL;
    }

    int direction = 0;
    if (border_settings.contains("wxLEFT"))
    {
        direction |= wxLEFT;
    }
    if (border_settings.contains("wxRIGHT"))
    {
        direction |= wxRIGHT;
    }
    if (border_settings.contains("wxTOP"))
    {
        direction |= wxTOP;
    }
    if (border_settings.contains("wxBOTTOM"))
    {
        direction |= wxBOTTOM;
    }
    return direction;
}

auto Node::ApplyAlignment(wxSizerFlags& flags, const wxue::string& alignment) -> void
{
    if (alignment.empty())
    {
        return;
    }

    if (alignment.contains("wxALIGN_CENTER"))
    {
        if (alignment.contains("wxALIGN_CENTER_VERTICAL") &&
            alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
        {
            flags.Center();
        }
        else if (alignment.contains("wxALIGN_CENTER_VERTICAL"))
        {
            flags.CenterVertical();
        }
        else if (alignment.contains("wxALIGN_CENTER_HORIZONTAL"))
        {
            flags.CenterHorizontal();
        }
        else
        {
            flags.Center();
        }
    }
    else
    {
        if (alignment.contains("wxALIGN_LEFT"))
        {
            flags.Left();
        }
        if (alignment.contains("wxALIGN_RIGHT"))
        {
            flags.Right();
        }
        if (alignment.contains("wxALIGN_TOP"))
        {
            flags.Top();
        }
        if (alignment.contains("wxALIGN_BOTTOM"))
        {
            flags.Bottom();
        }
    }
}

auto Node::ApplyAdditionalFlags(wxSizerFlags& flags, const wxue::string& prop) -> void
{
    if (prop.empty())
    {
        return;
    }

    if (prop.contains("wxEXPAND"))
    {
        flags.Expand();
    }
    if (prop.contains("wxSHAPED"))
    {
        flags.Shaped();
    }
    if (prop.contains("wxFIXED_MINSIZE"))
    {
        flags.FixedMinSize();
    }
    if (prop.contains("wxRESERVE_SPACE_EVEN_IF_HIDDEN"))
    {
        flags.ReserveSpaceEvenIfHidden();
    }
}

auto Node::getSizerFlags() const -> wxSizerFlags
{
    wxSizerFlags flags;
    flags.Proportion(as_int(prop_proportion));

    auto border_size = as_int(prop_border_size);
    int direction = GetBorderDirection(as_view(prop_borders));
    flags.Border(direction, border_size);

    ApplyAlignment(flags, as_string(prop_alignment));
    ApplyAdditionalFlags(flags, as_string(prop_flags));

    return flags;
}

auto Node::TryCreateInSizerChild(GenName name, bool verify_language_support, Node*& parent,
                                 NodeSharedPtr& new_node)
    -> std::pair<NodeSharedPtr, Node::Validity>
{
    if ((is_Form() || is_Container()) && get_ChildCount())
    {
        if (get_Child(0)->get_GenType() == type_sizer ||
            get_Child(0)->get_GenType() == type_gbsizer)
        {
            auto result = NodeCreation.CreateNode(name, get_Child(0), verify_language_support);
            if (!result.first || result.second != valid_node)
            {
                return { nullptr, result.second };
            }
            new_node = result.first;
            parent = get_Child(0);

            if (parent->get_GenType() == type_gbsizer)
            {
                return HandleGridBagInsertion(parent, new_node.get());
            }
        }
    }
    return { new_node, valid_node };
}

auto Node::HandleGridBagInsertion(Node* parent, Node* new_node)
    -> std::pair<NodeSharedPtr, Node::Validity>
{
    GridBag grid_bag(parent);
    if (grid_bag.InsertNode(parent, new_node))
    {
        return { new_node->get_SharedPtr(), valid_node };
    }
    return { nullptr, gridbag_insert_error };
}

auto Node::AdjustMemberNameForLanguage(Node* new_node) -> void
{
    if (!new_node->HasProp(prop_var_name))
    {
        return;
    }

    auto original_name = new_node->as_view(prop_var_name);
    std::string member_name;

    // Assign default name if empty
    if (original_name.empty())
    {
        member_name = new_node->get_PropDefaultValue(prop_var_name);
    }
    else
    {
        member_name = std::string(original_name);
    }

    const auto language = Project.get_CodePreference(this);

    // Apply C++ naming conventions
    if (language == GEN_LANG_CPLUSPLUS && UserPrefs.is_CppSnakeCase())
    {
        member_name = ConvertToSnakeCase(member_name);
    }

    // Remove m_ prefix (common in imported projects)
    if (member_name.starts_with("m_") && language != GEN_LANG_CPLUSPLUS)
    {
        member_name.erase(0, 2);

        // Remove duplicate suffix that was added assuming m_ prefix
        if (member_name.ends_with("_2"))
        {
            member_name.erase(member_name.size() - 2);
        }
    }

    // Apply Python private name convention
    if (language == GEN_LANG_PYTHON && new_node->is_Local())
    {
        member_name = "_" + member_name;
    }

    // Update node if name changed and check for duplicates
    if (member_name != original_name)
    {
        new_node->set_value(prop_var_name, member_name);
        new_node->FixDuplicateName();
    }
}

auto Node::HandleRibbonButtonFallback([[maybe_unused]] GenName name, int pos)
    -> std::pair<NodeSharedPtr, Node::Validity>
{
    auto result = NodeCreation.CreateNode(gen_ribbonTool, this);
    if (!result.first || result.second != valid_node)
    {
        return { nullptr, result.second };
    }
    auto new_node = result.first;
    wxue::string undo_str = "insert ribbon tool";
    wxGetFrame().PushUndoAction(
        std::make_shared<InsertNodeAction>(new_node.get(), this, undo_str, pos));
    return { new_node, valid_node };
}

auto Node::TryCreateInParent(GenName name, [[maybe_unused]] int pos)
    -> std::pair<NodeSharedPtr, Node::Validity>
{
    auto* parent = get_Parent();
    if (!parent)
    {
        wxMessageBox(wxue::string() << "You cannot add " << map_GenNames.at(name)
                                    << " as a child of " << get_DeclName());
        return { nullptr, invalid_child };
    }

    auto* decl = NodeCreation.get_declaration(name);
    auto max_children = get_NodeDeclaration()->get_AllowableChildren(decl->get_GenType());
    auto cur_children = NodeCreator::CountChildrenWithSameType(this, decl->get_GenType());
    if (max_children > 0 && cur_children >= static_cast<size_t>(max_children))
    {
        if (is_Gen(gen_wxSplitterWindow))
        {
            wxMessageBox("You cannot add more than two windows to a splitter window.",
                         "Cannot add control");
        }
        else
        {
            wxMessageBox(wxue::string()
                         << "You can only add " << (to_size_t) max_children << ' '
                         << map_GenNames.at(name) << " as a child of " << get_DeclName());
        }
        return { nullptr, invalid_child_count };
    }

    auto new_node = NodeCreation.CreateNode(name, parent).first;
    if (new_node)
    {
        if (parent->is_Gen(gen_wxGridBagSizer))
        {
            return HandleGridBagInsertion(parent, new_node.get());
        }

        auto insert_pos = parent->FindInsertionPos(this);
        wxue::string undo_str;
        undo_str << "insert " << map_GenNames.at(name);
        wxGetFrame().PushUndoAction(
            std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, insert_pos));
    }

    return { new_node, valid_node };
}

auto Node::CreateChildNode(GenName name, bool verify_language_support, int pos)
    -> std::pair<NodeSharedPtr, Node::Validity>
{
    auto& frame = wxGetFrame();

    auto result = NodeCreation.CreateNode(name, this, verify_language_support);
    if (!result.first || result.second != valid_node)
    {
        return { nullptr, result.second };
    }
    auto new_node = result.first;
    Node* parent = this;

    // If node creation failed, try creating in a sizer child or via fallback mechanisms
    if (!new_node)
    {
        new_node = NodeCreation.CreateNode(name, this).first;
        result = TryCreateInSizerChild(name, verify_language_support, parent, new_node);
        if (result.second != valid_node)
        {
            return result;
        }
    }

    // Handle successful node creation
    if (new_node)
    {
        if (is_Gen(gen_wxGridBagSizer))
        {
            return HandleGridBagInsertion(this, new_node.get());
        }

#ifdef _WIN32
        // In a Windows build, the default background colour of white doesn't match the normal
        // background color of the parent so we set it to the more normal Windows colour.
        if (name == gen_BookPage)
        {
            if (auto* prop = new_node->get_PropPtr(prop_background_colour); prop)
            {
                prop->set_value("wxSYS_COLOUR_BTNFACE");
            }
        }
#endif  // _WIN32

        wxue::string undo_str;
        undo_str << "insert " << map_GenNames.at(name);
        frame.PushUndoAction(
            std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    }
    // Handle ribbon button fallback
    else if (name == gen_ribbonButton)
    {
        return HandleRibbonButtonFallback(name, pos);
    }
    // Try creating in parent node
    else
    {
        result = TryCreateInParent(name, pos);
        if (result.second != valid_node)
        {
            return result;
        }
        new_node = result.first;
    }

    // Adjust variable name for target language
    if (new_node)
    {
        AdjustMemberNameForLanguage(new_node.get());
        frame.FireCreatedEvent(new_node.get());
        frame.SelectNode(new_node.get(), evt_flags::fire_event | evt_flags::force_selection);
    }

    return { new_node, valid_node };
}

auto Node::CreateNode(GenName name) -> Node*
{
    auto& frame = wxGetFrame();
    auto* cur_selection = frame.getSelectedNode();
    if (!cur_selection)
    {
        wxMessageBox("You need to select something first in order to properly place this widget.");
        return nullptr;
    }
    return cur_selection->CreateChildNode(name).first.get();
}

auto Node::ModifyProperty(PropName name, wxue::string_view value) -> void
{
    auto* prop = get_PropPtr(name);
    if (prop && value != prop->as_view())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

auto Node::ModifyProperty(wxue::string_view name, int value) -> void
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
    {
        prop = get_PropPtr(find_prop->second);
    }

    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

auto Node::ModifyProperty(wxue::string_view name, wxue::string_view value) -> void
{
    NodeProperty* prop = nullptr;
    if (auto find_prop = rmap_PropNames.find(name); find_prop != rmap_PropNames.end())
    {
        prop = get_PropPtr(find_prop->second);
    }

    if (prop && value != prop->as_string())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

auto Node::ModifyProperty(NodeProperty* prop, int value) -> void
{
    if (prop && value != prop->as_int())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

auto Node::ModifyProperty(NodeProperty* prop, wxue::string_view value) -> void
{
    if (prop && value != prop->as_view())
    {
        wxGetFrame().PushUndoAction(std::make_shared<ModifyPropertyAction>(prop, value));
    }
}

auto Node::get_UniqueName(const std::string& proposed_name, PropName prop_name) -> std::string
{
    std::string new_name(proposed_name);
    if (is_Form())
    {
        return {};
    }

    auto* form = get_Form();
    if (!form)
    {
        return {};
    }

    std::unordered_set<std::string> name_set;

    if (prop_name == prop_var_name)
    {
        for (const auto& iter: reserved_names)
        {
            name_set.emplace(iter);
        }

        form->CollectUniqueNames(name_set, this, prop_name);
    }
    else if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        if (auto* parent = get_Parent(); parent)
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

    if (auto iter = name_set.find(new_name); iter != name_set.end())
    {
        // We get here if the name has already been used.

        std::string org_name(proposed_name);
        while (wxue::is_digit(org_name.back()))
        {
            // remove any trailing digits
            org_name.erase(org_name.size() - 1, 1);
        }

        for (int i = 2; iter != name_set.end(); iter = name_set.find(new_name), ++i)
        {
            if (org_name.back() == '_')
            {
                org_name.pop_back();
            }
            new_name = org_name + std::to_string(i);
        }
    }

    // We return this name whether or not it has actually changed.
    return new_name;
}

static constexpr auto s_var_names = std::to_array<PropName>({

    prop_var_name,
    prop_checkbox_var_name,
    prop_radiobtn_var_name,
    prop_validator_variable,

});

auto Node::FixDuplicateName() -> bool
{
    if (is_Type(type_form) || is_Type(type_frame_form) || is_Type(type_menubar_form) ||
        is_Type(type_ribbonbar_form) || is_Type(type_toolbar_form) ||
        is_Type(type_aui_toolbar_form) || is_Type(type_panel_form) || is_Type(type_wizard) ||
        is_Type(type_popup_menu) || is_Type(type_project))
    {
        return false;
    }

    auto* form = get_Form();
    ASSERT(form || is_Folder());
    if (!form)
    {
        return false;
    }

    std::unordered_set<std::string> name_set;
    for (const auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }

    form->CollectUniqueNames(name_set, this);

    bool replaced = false;
    for (const auto& iter: s_var_names)
    {
        if (const auto& name = as_string(iter); name.size())
        {
            if (name_set.contains(name))
            {
                // We get here if the name has already been used.

                std::string org_name(name);
                while (wxue::is_digit(org_name.back()))
                {
                    // remove any trailing digits
                    org_name.erase(org_name.size() - 1, 1);
                }
                if (org_name.back() == '_')
                {
                    org_name.erase(org_name.size() - 1, 1);
                }

                std::string new_name;
                for (int i = 2; name_set.contains(new_name); ++i)
                {
                    new_name.clear();
                    new_name = org_name + std::to_string(i);
                }

                auto* fix_name = get_PropPtr(iter);
                fix_name->set_value(new_name);
                replaced = true;
            }
        }
    }

    if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        name_set.clear();
        form->CollectUniqueNames(name_set, this, prop_label);

        std::string org_name(as_string(prop_label));
        auto result = get_UniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto* fix_name = get_PropPtr(prop_label);
            fix_name->set_value(result);
        }
    }

    return replaced;
}

auto Node::FixDuplicateNodeNames(Node* form) -> void
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

        form = get_Form();

        ASSERT(form);
        if (!form)
        {
            return;
        }
    }

    // Collect all variable names except for those in the current node. Then check if any of
    // the variable names in the current node are duplicated in other nodes, and if so fix
    // them. Then step into each of the child nodes recursively until all nodes have been
    // processed. The reason for collecting the names for each set is because there could be
    // multiple identical names if the node was generated by one of the CreateNew functions.
    // Even pasting in from wxSmith or wxFormBuilder could have multiple identical names if
    // they didn't do their own name fixups correctly.

    std::unordered_set<std::string> name_set;
    InitializeNameSet(name_set);
    form->CollectUniqueNames(name_set, this);

    FixDuplicateVariableNames(name_set);
    FixChildrenNodeNames(form);
    FixPropGridLabelIfNeeded();
}

auto Node::InitializeNameSet(std::unordered_set<std::string>& name_set) -> void
{
    for (const auto& iter: reserved_names)
    {
        name_set.emplace(iter);
    }
}

auto Node::GenerateUniqueNameFromBase(const std::string& base_name,
                                      const std::unordered_set<std::string>& name_set)
    -> std::string
{
    std::string org_name(base_name);
    while (wxue::is_digit(org_name.back()))
    {
        // remove any trailing digits
        org_name.erase(org_name.size() - 1, 1);
    }
    if (org_name.back() == '_')
    {
        org_name.erase(org_name.size() - 1, 1);
    }

    std::string new_name(org_name);
    for (int i = 2; name_set.contains(new_name); ++i)
    {
        new_name.clear();
        new_name = org_name + std::to_string(i);
    }

    return new_name;
}

auto Node::FixDuplicateVariableNames(const std::unordered_set<std::string>& name_set) -> void
{
    for (const auto& iter: s_var_names)
    {
        if (const auto& name = as_string(iter); name.size())
        {
            if (name_set.contains(name))
            {
                // We get here if the name has already been used.
                auto new_name = GenerateUniqueNameFromBase(name, name_set);
                auto* fix_name = get_PropPtr(iter);
                fix_name->set_value(new_name);
            }
        }
    }
}

auto Node::FixChildrenNodeNames(Node* form) -> void
{
    for (const auto& child: get_ChildNodePtrs())
    {
        child->FixDuplicateNodeNames(form);
    }
}

auto Node::FixPropGridLabelIfNeeded() -> void
{
    if (is_Gen(gen_propGridItem) || is_Gen(gen_propGridCategory))
    {
        std::unordered_set<std::string> name_set;
        if (auto* parent = get_Parent(); parent)
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

        std::string org_name(as_string(prop_label));
        auto result = get_UniqueName(org_name, prop_label);
        if (result != as_string(prop_label))
        {
            auto* fix_name = get_PropPtr(prop_label);
            fix_name->set_value(result);
        }
    }
}

auto Node::CollectUniqueNames(std::unordered_set<std::string>& name_set, Node* cur_node,
                              PropName prop_name) -> void
{
    if (!is_Form() && cur_node != this && !is_Gen(gen_wxPropertyGrid) &&
        !is_Gen(gen_wxPropertyGridManager))
    {
        if (prop_name == prop_var_name)
        {
            for (const auto& iter: s_var_names)
            {
                // First check the parent node
                if (const auto& name = as_string(iter); name.size())
                {
                    name_set.emplace(name);
                }

                // Now check the child node, skipping prop_var_name. This is needed in order to
                // pick up validator names.
                if (iter != prop_var_name)
                {
                    if (const auto& name = cur_node->as_string(iter); name.size())
                    {
                        name_set.emplace(name);
                    }
                }
            }
        }
        else
        {
            if (const auto& name = as_string(prop_name); name.size())
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

auto Node::FindInsertionPos(Node* child) const -> ptrdiff_t
{
    if (child)
    {
        for (size_t new_pos = 0; new_pos < get_ChildCount(); ++new_pos)
        {
            if (get_Child(new_pos) == child)
            {
                return static_cast<ptrdiff_t>(new_pos + 1);
            }
        }
    }
    return -1;
}

auto Node::get_NodeSize() const -> size_t
{
    auto size = sizeof(*this);
    // Add the size of all the node pointers, but not the size of the individual children
    size += (m_children.size() * sizeof(void*));

    for (const auto& iter: m_properties)
    {
        size += iter.get_PropSize();
    }

    for (const auto& iter: m_map_events)
    {
        size += iter.second.get_EventSize();
    }

    size += (m_prop_indices.size() * (sizeof(size_t) * 2));

    return size;
}

// Create a hash of the node name and all property values of the node, and recursively call all
// children
auto Node::CalcNodeHash(size_t& hash) const -> void
{
    // djb2 hash algorithm

    if (hash == 0)
    {
        hash = 5381;
    }

    for (auto iter: get_NodeName())
    {
        hash = ((hash << 5) + hash) ^ iter;
    }

    for (const auto& prop: m_properties)
    {
        for (auto char_iter: prop.as_string())
        {
            hash = ((hash << 5) + hash) ^ char_iter;
        }
    }

    for (const auto& child: m_children)
    {
        child->CalcNodeHash(hash);
    }
}

auto Node::FindAllChildProperties(PropName name) -> std::vector<NodeProperty*>
{
    std::vector<NodeProperty*> result;

    FindAllChildProperties(result, name);

    return result;
}

auto Node::FindAllChildProperties(std::vector<NodeProperty*>& list, PropName name) -> void
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

static constexpr frozen::set<GenEnum::GenName, 5> s_bool_validators = {

    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,
    gen_wxCheckBox,
    gen_wxRadioButton,
    gen_wxToggleButton,
};

static constexpr frozen::set<GenEnum::GenName, 7> s_int_validators = {

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
static constexpr frozen::set<GenEnum::GenName, 4> s_read_only_validators = {

    gen_wxButton,
    gen_wxComboBox,  // get should still work, unlike button and static text
    gen_wxStaticText,
    gen_wxTextCtrl,

};

// These use a wxArrayInt variable
static constexpr frozen::set<GenEnum::GenName, 2> s_array_int_validators = {

    gen_wxCheckListBox,
    gen_wxListBox,

};

// These use a wxDatePickerCtrl variable
static constexpr frozen::set<GenEnum::GenName, 1> s_date_picker_validators = {

    gen_wxDatePickerCtrl,

};

// clang-format on

auto Node::get_ValidatorDataType() const -> std::string
{
    std::string data_type;
    if (HasProp(prop_validator_data_type))
    {
        data_type = as_view(prop_validator_data_type);
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

auto Node::get_ValidatorType() const -> std::string_view
{
    if (!is_Gen(gen_wxTextCtrl))
    {
        return "wxGenericValidator";
    }

    auto data_type = as_view(prop_validator_data_type);
    if (data_type == "wxString")
    {
        return "wxTextValidator";
    }
    if (data_type == "int" || data_type == "short" || data_type == "long" ||
        data_type == "long long" || data_type == "unsigned int" || data_type == "unsigned short" ||
        data_type == "unsigned long" || data_type == "unsigned long long")
    {
        return "wxIntegerValidator";
    }
    if (data_type == "double" || data_type == "float")
    {
        return "wxFloatingPointValidator";
    }
    return "wxGenericValidator";
}

auto Node::get_PlatformContainer() -> Node*  // NOLINT (cppcheck-suppress)
{
    if (auto* parent = get_Parent(); parent)
    {
        while (parent && !parent->is_Gen(gen_Project))
        {
            if (parent->HasProp(prop_platforms) &&
                parent->as_view(prop_platforms) != "Windows|Unix|Mac")
            {
                return parent;
            }
            parent = parent->get_Parent();
        }
    }
    return nullptr;
}
