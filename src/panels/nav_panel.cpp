////////////////////////////////////////////////////////////////////////////
// Purpose:   Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // for std::ranges::any_of
#include <tuple>      // for std::ignore

#include <wx/imaglist.h>  // wxImageList base header
#include <wx/infobar.h>   // declaration of wxInfoBarBase defining common API of wxInfoBar
#include <wx/panel.h>     // Base header for wxPanel
#include <wx/sizer.h>     // provide wxSizer class for layout
#include <wx/toolbar.h>   // wxToolBar interface declaration
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "nav_panel.h"

#include "bitmaps.h"          // Contains various images handling functions
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "mainframe_base.h"   // contains all the wxue_img namespace embedded images
#include "nav_toolbar.h"      // generated NavToolbar class
#include "navpopupmenu.h"     // NavPopupMenu -- Context-menu for an item
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "node_decl.h"        // NodeDeclaration class
#include "preferences.h"      // Prefs -- Set/Get wxUiEditor preferences
#include "project_handler.h"  // ProjectHandler class
#include "propgrid_panel.h"   // PropGridPanel -- PropertyGrid class for node properties and events
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties
#include "wxue_namespace/wxue_view_vector.h"  // wxue::ViewVector
#include "wxue_string.h"

constexpr size_t MAX_LABEL_LENGTH = 24;

// clang-format off
inline const std::vector<GenEnum::GenName> unsupported_gen_python = {

    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,

    gen_wxContextMenuEvent,

};

inline const std::vector<GenEnum::GenName> unsupported_gen_ruby = {

};

inline const std::vector<GenEnum::GenName> unsupported_gen_XRC = {

    gen_TreeListCtrlColumn,
    gen_propGridCategory,
    gen_propGridItem,
    gen_propGridPage,
    gen_wxContextMenuEvent,
    gen_wxPopupTransientWindow,
    gen_wxPropertyGrid,
    gen_wxPropertyGridManager,
    gen_wxRearrangeCtrl,
    gen_wxTreeListCtrl,
    gen_wxWebView,

};
// clang-format on

NavigationPanel::NavigationPanel(wxWindow* parent) : wxPanel(parent)
{
    SetWindowStyle(wxBORDER_RAISED);

    m_tree_ctrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxTR_DEFAULT_STYLE | wxBORDER_SUNKEN);

    int index = 0;
    const wxSize gen_image_size =
        parent->FromDIP(wxSize(UserPrefs.get_IconSize(), UserPrefs.get_IconSize()));

    wxVector<wxBitmapBundle> bundles;
    for (const auto& iter: NodeCreation.get_NodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }

        if (const wxBitmapBundle bundle = iter->GetBitmapBundle(gen_image_size.x, gen_image_size.y);
            bundle.IsOk())
        {
            bundles.push_back(bundle);
            m_icon_index[iter->get_GenName()] = index++;
        }
    }
    bundles.push_back(GetSvgImage("svg", gen_image_size.x, gen_image_size.y));
    m_icon_index[gen_svg_embedded_image] = index++;
    m_tree_ctrl->SetImages(bundles);

    m_toolbar = new NavToolbar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);
    m_toolbar->Realize();

    auto* toolbar_sizer = new wxBoxSizer(wxVERTICAL);
    toolbar_sizer->Add(m_toolbar, wxSizerFlags().Expand());

    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);
    parent_sizer->Add(toolbar_sizer, wxSizerFlags().Expand());
    parent_sizer->Add(m_tree_ctrl, wxSizerFlags(1).Expand());

    SetSizerAndFit(parent_sizer);

    Bind(wxEVT_TREE_SEL_CHANGED, &NavigationPanel::OnSelChanged, this);

    Bind(wxEVT_TREE_ITEM_RIGHT_CLICK, &NavigationPanel::OnRightClick, this);
    Bind(wxEVT_TREE_BEGIN_DRAG, &NavigationPanel::OnBeginDrag, this);
    Bind(wxEVT_TREE_END_DRAG, &NavigationPanel::OnEndDrag, this);

    Bind(EVT_NodePropChange, &NavigationPanel::OnNodePropChange, this);
    Bind(EVT_MultiPropChange, &NavigationPanel::OnMultiPropChange, this);
    Bind(EVT_NodeSelected, &NavigationPanel::OnNodeSelected, this);
    Bind(EVT_ParentChanged, &NavigationPanel::OnParentChange, this);
    Bind(EVT_PositionChanged, &NavigationPanel::OnPositionChange, this);

    Bind(EVT_ProjectUpdated,
         [this](CustomEvent&)
         {
             OnProjectUpdated();
         });

    Bind(EVT_NodeCreated, &NavigationPanel::OnNodeCreated, this);
    Bind(EVT_NodeDeleted,
         [this](CustomEvent& event)
         {
             DeleteNode(event.getNode());
         });

    Bind(wxEVT_MENU, &NavigationPanel::OnExpand, this, NavToolbar::id_NavExpand);
    Bind(wxEVT_MENU, &NavigationPanel::OnCollapse, this, NavToolbar::id_NavCollapse);
    Bind(wxEVT_MENU, &NavigationPanel::OnCollExpand, this, NavToolbar::id_NavCollExpand);

    Bind(wxEVT_UPDATE_UI, &NavigationPanel::OnUpdateEvent, this);
}

void NavigationPanel::SetMainFrame(MainFrame* frame)
{
    m_pMainFrame = frame;

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            m_pMainFrame->MoveNode(MoveDirection::Down);
        },
        NavToolbar::id_NavMoveDown);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            m_pMainFrame->MoveNode(MoveDirection::Left);
        },
        NavToolbar::id_NavMoveLeft);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            m_pMainFrame->MoveNode(MoveDirection::Right);
        },
        NavToolbar::id_NavMoveRight);

    Bind(
        wxEVT_MENU,
        [this](wxCommandEvent&)
        {
            m_pMainFrame->MoveNode(MoveDirection::Up);
        },
        NavToolbar::id_NavMoveUp);

    m_pMainFrame->AddCustomEventHandler(GetEventHandler());
}

Node* NavigationPanel::GetNode(wxTreeItemId item)
{
    if (item.IsOk())
    {
        if (const TreeNodeMap::iterator result = m_tree_node_map.find(item);
            result != m_tree_node_map.end())
        {
            return result->second;
        }
    }
    return nullptr;
}

void NavigationPanel::OnProjectUpdated()
{
    const wxWindowUpdateLocker freeze(this);

#if defined(_DEBUG)
    if (App::isFireCreationMsgs())
    {
        MSG_INFO("Navigation tree control recreated.");
    }
#endif  // _DEBUG

    m_tree_ctrl->DeleteAllItems();
    m_tree_node_map.clear();
    m_node_tree_map.clear();

    const wxTreeItemId root = m_tree_ctrl->AddRoot(GetDisplayName(Project.get_ProjectNode()),
                                                   GetImageIndex(Project.get_ProjectNode()), -1);
    m_node_tree_map[Project.get_ProjectNode()] = root;
    m_tree_node_map[root] = Project.get_ProjectNode();

    AddAllChildren(Project.get_ProjectNode());

    // First we expand everything, then we collapse all forms and folders
    ExpandAllNodes(Project.get_ProjectNode());

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (const NodeTreeMap::iterator result = m_node_tree_map.find(form);
            result != m_node_tree_map.end())
        {
            m_tree_ctrl->Collapse(result->second);
        }
    }

    for (const auto& folder: Project.get_ChildNodePtrs())
    {
        if (folder->is_Gen(gen_folder))
        {
            if (const NodeTreeMap::iterator result = m_node_tree_map.find(folder.get());
                result != m_node_tree_map.end())
            {
                m_tree_ctrl->Collapse(result->second);
            }
        }
    }
}

void NavigationPanel::OnSelChanged(wxTreeEvent& event)
{
    if (m_isSelChangeSuspended)
    {
        return;
    }

    const wxTreeItemId tree_item = event.GetItem();
    if (!tree_item.IsOk())
    {
        return;
    }

    if (const TreeNodeMap::iterator iter = m_tree_node_map.find(tree_item);
        iter != m_tree_node_map.end())
    {
        // Selecting a node can result in multiple selection events getting fired as the
        // Mockup selects the current dialog, and the current book or page. In some cases a
        // generator will also fire off a selection event when mockup sets the current page
        // (e.g., wxEVT_NOTEBOOK_PAGE_CHANGED). There's no reason for the property grid to
        // update itself until we're done, so we lock it before the initial selection event.

        m_isSelChangeSuspended = true;
        m_pMainFrame->get_PropPanel()->Lock();
        m_pMainFrame->SelectNode(iter->second);
        m_pMainFrame->get_PropPanel()->UnLock();

        // It's possible for Mockup to select a page, so we need to be certain everything is
        // synced after the initial selection.

        if (iter->second != m_pMainFrame->getSelectedNode())
        {
            m_pMainFrame->SelectNode(iter->second);
        }
        else
        {
            m_pMainFrame->get_PropPanel()->Create();
        }

        // TODO: [Randalphwa - 09-30-2024] Once all generators support isLanguageVersionSupported(),
        // this should be changed to call the generator to determine if the control is
        // supported by the current language.

        if (Project.get_CodePreference() == GEN_LANG_PYTHON)
        {
            const GenName gen_name = iter->second->get_GenName();
            if (std::ranges::any_of(unsupported_gen_python,
                                    [gen_name](const GenName& name)
                                    {
                                        return name == gen_name;
                                    }))
            {
                wxGetFrame().ShowInfoBarMsg("This control is not supported by wxPython.",
                                            wxICON_INFORMATION);
            }
        }
        if (Project.get_CodePreference() == GEN_LANG_RUBY)
        {
            const GenName gen_name = iter->second->get_GenName();
            if (std::ranges::any_of(unsupported_gen_ruby,
                                    [gen_name](const GenName& name)
                                    {
                                        return name == gen_name;
                                    }))
            {
                wxGetFrame().ShowInfoBarMsg("This control is not supported by wxRuby.",
                                            wxICON_INFORMATION);
            }
        }
        if (Project.as_string(prop_code_preference) == "XRC")
        {
            const GenName gen_name = iter->second->get_GenName();
            if (std::ranges::any_of(unsupported_gen_XRC,
                                    [gen_name](const GenName& name)
                                    {
                                        return name == gen_name;
                                    }))
            {
                wxGetFrame().ShowInfoBarMsg("This control is not supported by XRC.",
                                            wxICON_INFORMATION);
            }
        }

        m_isSelChangeSuspended = false;
    }
}

void NavigationPanel::OnRightClick(wxTreeEvent& event)
{
    const wxTreeItemId tree_item = event.GetItem();
    if (const TreeNodeMap::iterator iter = m_tree_node_map.find(tree_item);
        iter != m_tree_node_map.end())
    {
        auto* menu = new NavPopupMenu(iter->second);
        const wxPoint pos = event.GetPoint();
        menu->UpdateUI(menu);
        PopupMenu(menu, pos.x, pos.y);
    }
}

void NavigationPanel::OnBeginDrag(wxTreeEvent& event)
{
    if (event.GetItem() == m_tree_ctrl->GetRootItem())
    {
        return;
    }

    m_drag_node = event.GetItem();
    const Node* node = GetNode(m_drag_node);
    if (node)
    {
        const Node* parent = node->get_Parent();
        if (node->is_Gen(gen_Images) || node->is_Gen(gen_Data) || parent->is_Gen(gen_Images) ||
            parent->is_Gen(gen_Data))
        {
            event.Veto();
            return;
        }
    }
    event.Allow();
}

void NavigationPanel::OnEndDrag(wxTreeEvent& event)
{
    const wxTreeItemId item_src = m_drag_node;
    m_drag_node = nullptr;

    const wxTreeItemId item_dst = event.GetItem();

    // ensure that item_dst is not item_src or a child of item_src
    wxTreeItemId item = item_dst;
    while (item.IsOk())
    {
        if (item == item_src)
        {
            if (wxMessageBox("Do you want to duplicate this item?", "Drop item onto itself",
                             wxYES_NO) == wxYES)
            {
                wxGetFrame().DuplicateNode(GetNode(item_src));
                ExpandAllNodes(wxGetFrame().getSelectedNode());
            }
            return;
        }
        item = m_tree_ctrl->GetItemParent(item);
    }

    Node* node_src = GetNode(item_src);
    ASSERT(node_src);
    if (!node_src)
    {
        return;
    }

    Node* node_dst = GetNode(item_dst);
    ASSERT(node_dst);
    if (!node_dst)
    {
        return;
    }

    if (node_dst->is_Gen(gen_wxSplitterWindow) && node_dst->get_ChildCount() > 1)
    {
        // If the user drags and drops windows within a wxSplitterWindow, then just swap places
        if (node_dst->get_Child(1) == node_src)
        {
            m_pMainFrame->MoveNode(node_src, MoveDirection::Up);
            return;
        }
        if (node_dst->get_Child(0) == node_src)
        {
            m_pMainFrame->MoveNode(node_src, MoveDirection::Down);
            return;
        }
        if (node_dst->get_ChildCount() == 2)
        {
            wxMessageBox("A wxSplitterWindow can't have more than two windows.");
            return;
        }
    }

    Node* dst_parent = node_dst;
    while (!dst_parent->is_ChildAllowed(node_src))
    {
        if (dst_parent->is_Sizer())
        {
            const std::string_view decl_name = node_src->get_DeclName();
            wxMessageBox(wxString("You can't drop a ")
                         << wxString(decl_name.data(), decl_name.size()) << " onto a sizer.");
            return;
        }
        if (dst_parent->is_Container())
        {
            const std::string_view src_name = node_src->get_DeclName();
            const std::string_view dst_name = dst_parent->get_DeclName();
            wxMessageBox(wxString("You can't drop a ")
                         << wxString(src_name.data(), src_name.size()) << " onto a "
                         << wxString(dst_name.data(), dst_name.size()) << '.');
            return;
        }
        if (dst_parent->is_Gen(gen_Project))
        {
            wxMessageBox("Only forms can be dropped onto your project.");
            return;
        }
        dst_parent = dst_parent->get_Parent();
        if (!dst_parent)
        {
            wxMessageBox(wxString(node_src->get_DeclName().data(), node_src->get_DeclName().size())
                         << " can't be dropped onto this target.");
            return;
        }
    }

    if (dst_parent->is_Gen(gen_wxStdDialogButtonSizer))
    {
        wxMessageBox(wxString("You can't drop a ")
                     << wxString(node_src->get_DeclName().data(), node_src->get_DeclName().size())
                     << " onto a wxStdDialogBtnSizer.");
        return;
    }

    const Node* src_parent = node_src->get_Parent();
    if (dst_parent->is_Gen(gen_wxGridBagSizer))
    {
        if (src_parent == dst_parent)
        {
            wxMessageBox("You cannot drag and drop an item within the same wxGridBagSizer. Use the "
                         "Move commands instead.");
            return;
        }
    }
    if (src_parent == dst_parent)
    {
        m_pMainFrame->PushUndoAction(std::make_shared<ChangePositionAction>(
            node_src, dst_parent->get_ChildPosition(node_dst)));
        return;
    }

    m_pMainFrame->PushUndoAction(std::make_shared<ChangeParentAction>(node_src, dst_parent));
}

void NavigationPanel::OnNodeCreated(CustomEvent& event)
{
    const wxWindowUpdateLocker freeze(this);
    InsertNode(event.getNode());
}

void NavigationPanel::RefreshParent(Node* parent)
{
    const wxWindowUpdateLocker freeze(this);
    for (const auto& child: parent->get_ChildNodePtrs())
    {
        EraseAllMaps(child.get());
    }
    AddAllChildren(parent);
    ExpandAllNodes(parent);
}

void NavigationPanel::InsertNode(Node* node)
{
    Node* node_parent = node->get_Parent();
    ASSERT(node_parent);
    const wxTreeItemId tree_parent = m_node_tree_map[node_parent];
    ASSERT(tree_parent);
    const wxTreeItemId new_item =
        m_tree_ctrl->InsertItem(tree_parent, node_parent->get_ChildPosition(node),
                                GetDisplayName(node).wx(), GetImageIndex(node), -1);
    m_node_tree_map[node] = new_item;
    m_tree_node_map[new_item] = node;

    if (node->get_ChildCount())
    {
        AddAllChildren(node);
        ExpandAllNodes(node);
    }
    else if (node->get_Parent() && (node->get_Parent()->is_Type(type_toolbar) ||
                                    node->get_Parent()->is_Type(type_aui_toolbar)))
    {
        // Insure that the toolbar is expanded when a new item is added to it
        ChangeExpansion(node->get_Parent(), false, true);
    }
}

void NavigationPanel::AddAllChildren(Node* node_parent)
{
    const wxTreeItemId tree_parent = m_node_tree_map[node_parent];
    ASSERT(tree_parent.IsOk());

    for (const auto& iter_child: node_parent->get_ChildNodePtrs())
    {
        Node* node = iter_child.get();
        const wxTreeItemId new_item = m_tree_ctrl->AppendItem(
            tree_parent, GetDisplayName(node).wx(), GetImageIndex(node), -1);

        m_node_tree_map[node] = new_item;
        m_tree_node_map[new_item] = node;

        if (node->get_ChildCount())
        {
            AddAllChildren(node);
        }
    }
}

int NavigationPanel::GetImageIndex(Node* node)
{
    GenName name = node->get_GenName();
    if (node->is_Gen(gen_wxBoxSizer))
    {
        if (node->is_PropValue(prop_orientation, "wxVERTICAL"))
        {
            name = gen_VerticalBoxSizer;
        }
    }

    if (node->is_Gen(gen_embedded_image))
    {
        if (node->as_string(prop_bitmap).starts_with("SVG"))
        {
            if (const IconIndexMap::iterator found_iter = m_icon_index.find(gen_svg_embedded_image);
                found_iter != m_icon_index.end())
            {
                return found_iter->second;
            }
        }
    }

    if (const IconIndexMap::iterator found_iter = m_icon_index.find(name);
        found_iter != m_icon_index.end())
    {
        return found_iter->second;
    }
    return 0;
}

void NavigationPanel::UpdateDisplayName(wxTreeItemId tree_item, Node* node)
{
    m_tree_ctrl->SetItemText(tree_item, GetDisplayName(node).wx());
}

wxue::string NavigationPanel::GetDisplayName(Node* node) const
{
    wxue::string display_name;
    if (node->HasValue(prop_label))
    {
        display_name = node->as_string(prop_label);
    }
    else if (node->HasValue(prop_main_label))  // used by wxCommandLinkButton
    {
        display_name = node->as_string(prop_main_label);
    }
    else if (node->HasValue(prop_var_name))
    {
        display_name = node->as_string(prop_var_name);
    }
    else if (node->HasValue(prop_id) && node->is_Gen(gen_ribbonTool))
    {
        display_name = node->as_string(prop_id);
    }
    else if (node->is_Gen(gen_embedded_image))
    {
        constexpr size_t IndexType = 0;
        constexpr size_t IndexImage = 1;
        if (!node->as_string(prop_bitmap).contains(";"))
        {
            display_name = node->as_string(prop_bitmap).filename();
        }
        else
        {
            wxue::ViewVector mstr(node->as_string(prop_bitmap), ';');
            if (mstr.size() > IndexImage)
            {
                display_name = mstr[IndexImage].filename();

                if (mstr[IndexType].is_sameas("Art"))
                {
                    display_name.erase_from('|');
                }
            }
        }
    }

    if (display_name.size())
    {
        // Accelerators make the text hard to read, so remove them
        std::ignore = display_name.Replace("&", "", true);

        if (!node->is_Form() && display_name.size() > MAX_LABEL_LENGTH)
        {
            display_name.erase(MAX_LABEL_LENGTH);
            display_name << "...";
        }
    }
    else
    {
        if (node->is_Gen(gen_Project))
        {
            display_name << "Project: " << Project.get_ProjectFile().filename();
        }
        else if (node->is_Gen(gen_wxContextMenuEvent))
        {
            display_name = node->as_string(prop_handler_name);
            if (display_name.size() > MAX_LABEL_LENGTH)
            {
                display_name.erase(MAX_LABEL_LENGTH);
                display_name << "...";
            }
        }
        else
        {
            display_name << " (" << node->get_DeclName() << ")";
        }
    }

    return display_name;
}

void NavigationPanel::ExpandAllNodes(Node* node)
{
    const wxWindowUpdateLocker freeze(this);
    if (const NodeTreeMap::iterator item_it = m_node_tree_map.find(node);
        item_it != m_node_tree_map.end())
    {
        if (m_tree_ctrl->ItemHasChildren(item_it->second))
        {
            m_tree_ctrl->Expand(item_it->second);
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        ExpandAllNodes(child.get());
    }
}

void NavigationPanel::DeleteNode(Node* node)
{
    const wxWindowUpdateLocker freeze(this);
    EraseAllMaps(node);
}

void NavigationPanel::EraseAllMaps(Node* node)
{
    // If you delete a parent tree item it will automatically delete all children, but our maps
    // won't reflect that. To keep the treeview control and our maps in sync, we need to delete
    // children before we delete the actual item.

    for (const auto& child: node->get_ChildNodePtrs())
    {
        EraseAllMaps(child.get());
    }

    if (const NodeTreeMap::iterator result = m_node_tree_map.find(node);
        result != m_node_tree_map.end())
    {
        m_tree_node_map.erase(result->second);
        if (result->second.IsOk())
        {
            m_tree_ctrl->Delete(result->second);
        }

        // Don't erase this until the iterator is no longer needed
        m_node_tree_map.erase(node);
    }
}
void NavigationPanel::OnNodeSelected(CustomEvent& event)
{
    Node* const node = event.getNode();
    if (node->get_Parent() && node->get_Parent()->is_Gen(gen_wxGridBagSizer))
    {
        wxGetFrame().setStatusText(wxString() << "Row: " << node->as_int(prop_row)
                                              << ", Column: " << node->as_int(prop_column));
    }
    else
    {
        if (node->HasValue(prop_var_name) &&
            !node->as_string(prop_class_access).starts_with("none"))
        {
            wxGetFrame().setStatusText(node->as_string(prop_var_name));
        }
        else
        {
            wxGetFrame().setStatusText(wxue::wxue_empty_string);
        }
    }

    if (m_isSelChangeSuspended)
    {
        return;
    }

    if (const NodeTreeMap::iterator iter = m_node_tree_map.find(node);
        iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
    else
    {
        FAIL_MSG(wxString("There is no tree item associated with this object.\n\tClass: ")
                 << wxString(node->get_DeclName().data(), node->get_DeclName().size())
                 << "\n\tName: " << node->as_string(prop_var_name));
    }
}

void NavigationPanel::OnMultiPropChange(CustomEvent& event)
{
    const std::vector<ModifyProperties::MULTI_PROP>& properties =
        static_cast<ModifyProperties*>(event.GetUndoCmd())->GetVector();
    for (const ModifyProperties::MULTI_PROP& property: properties)
    {
        CustomEvent prop_event(EVT_NodePropChange, property.property);
        OnNodePropChange(prop_event);
    }
}

void NavigationPanel::OnNodePropChange(CustomEvent& event)
{
    NodeProperty* prop = event.GetNodeProperty();

    if (prop->isProp(prop_var_name) || prop->isProp(prop_label) || prop->isProp(prop_class_name) ||
        prop->isProp(prop_bitmap))
    {
        const std::string_view class_name = prop->getNode()->get_DeclName();
        if (class_name.contains("bookpage"))
        {
            if (const NodeTreeMap::iterator found_iter =
                    m_node_tree_map.find(prop->getNode()->get_Child(0));
                found_iter != m_node_tree_map.end())
            {
                UpdateDisplayName(found_iter->second, found_iter->first);
            }
        }
        else if (const NodeTreeMap::iterator found_iter = m_node_tree_map.find(prop->getNode());
                 found_iter != m_node_tree_map.end())
        {
            UpdateDisplayName(found_iter->second, found_iter->first);
        }
    }
    else if (prop->isProp(prop_id) && prop->getNode()->is_Gen(gen_ribbonTool))
    {
        if (const NodeTreeMap::iterator found_iter = m_node_tree_map.find(prop->getNode());
            found_iter != m_node_tree_map.end())
        {
            UpdateDisplayName(found_iter->second, found_iter->first);
        }
    }
    else if (prop->isProp(prop_orientation))
    {
        if (const NodeTreeMap::iterator found_iter = m_node_tree_map.find(prop->getNode());
            found_iter != m_node_tree_map.end())
        {
            if (found_iter->first->is_Gen(gen_VerticalBoxSizer) ||
                found_iter->first->is_Gen(gen_wxBoxSizer))
            {
                const int image_index = GetImageIndex(found_iter->first);
                m_tree_ctrl->SetItemImage(found_iter->second, image_index);
            }
        }
    }
    else if (prop->isProp(prop_handler_name))
    {
        if (const NodeTreeMap::iterator found_iter = m_node_tree_map.find(prop->getNode());
            found_iter != m_node_tree_map.end())
        {
            UpdateDisplayName(found_iter->second, found_iter->first);
        }
    }
    else if (prop->isProp(prop_bitmap) && prop->getNode()->is_Gen(gen_embedded_image))
    {
        if (const NodeTreeMap::iterator found_iter = m_node_tree_map.find(prop->getNode());
            found_iter != m_node_tree_map.end())
        {
            UpdateDisplayName(found_iter->second, found_iter->first);
        }
    }
}

void NavigationPanel::OnUpdateEvent(wxUpdateUIEvent& event)
{
    Node* node = m_pMainFrame->getSelectedNode();
    if (!node)
    {
        event.Enable(false);
        return;
    }

    switch (event.GetId())
    {
        case NavToolbar::id_NavMoveUp:
            event.Enable(m_pMainFrame->MoveNode(node, MoveDirection::Up, true));
            break;

        case NavToolbar::id_NavMoveDown:
            event.Enable(m_pMainFrame->MoveNode(node, MoveDirection::Down, true));
            break;

        case NavToolbar::id_NavMoveLeft:
            event.Enable(m_pMainFrame->MoveNode(node, MoveDirection::Left, true));
            break;

        case NavToolbar::id_NavMoveRight:
            event.Enable(m_pMainFrame->MoveNode(node, MoveDirection::Right, true));
            break;

        case NavToolbar::id_NavExpand:
            event.Enable(node->get_ChildCount() > 0);
            break;

        case NavToolbar::id_NavCollapse:
            event.Enable(node->get_Parent() && node->get_Parent()->get_ChildCount() > 0);
            break;

        case NavToolbar::id_NavCollExpand:
            event.Enable((node->get_Parent() && node->get_Parent()->get_ChildCount() > 0) ||
                         node->get_ChildCount() > 0);
            break;

        default:
            break;
    }
}

void NavigationPanel::OnParentChange(CustomEvent& event)
{
    const wxWindowUpdateLocker freeze(this);

    auto* undo_cmd = dynamic_cast<ChangeParentAction*>(event.GetUndoCmd());

    m_isSelChangeSuspended = true;
    m_tree_ctrl->Unselect();
    EraseAllMaps(undo_cmd->getNode());
    InsertNode(undo_cmd->getNode());
    m_isSelChangeSuspended = false;

    if (const NodeTreeMap::iterator iter = m_node_tree_map.find(m_pMainFrame->getSelectedNode());
        iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
}

void NavigationPanel::OnPositionChange(CustomEvent& event)
{
    const wxWindowUpdateLocker freeze(this);

    auto* undo_cmd = dynamic_cast<ChangePositionAction*>(event.GetUndoCmd());

    m_isSelChangeSuspended = true;
    m_tree_ctrl->Unselect();
    EraseAllMaps(undo_cmd->getNode());
    InsertNode(undo_cmd->getNode());
    m_isSelChangeSuspended = false;

    if (const NodeTreeMap::iterator iter = m_node_tree_map.find(m_pMainFrame->getSelectedNode());
        iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
}

void NavigationPanel::ChangeExpansion(Node* node, bool include_children, bool expand)
{
    if (include_children)
    {
        for (const auto& child: node->get_ChildNodePtrs())
        {
            if (child->get_ChildCount())
            {
                ChangeExpansion(child.get(), include_children, expand);
            }
        }
    }
    if (node->get_ChildCount())
    {
        if (expand)
        {
            if (const NodeTreeMap::iterator item_iter = m_node_tree_map.find(node);
                item_iter != m_node_tree_map.end())
            {
                m_tree_ctrl->Expand(item_iter->second);
            }
        }
        else
        {
            if (const NodeTreeMap::iterator item_iter = m_node_tree_map.find(node);
                item_iter != m_node_tree_map.end())
            {
                m_tree_ctrl->Collapse(item_iter->second);
            }
        }
    }
}

void NavigationPanel::OnExpand(wxCommandEvent& /* event */)
{
    ASSERT(m_pMainFrame->getSelectedNode());
    Node* node = m_pMainFrame->getSelectedNode();
    if (!node)
    {
        return;  // This is theoretically impossible
    }

    const wxWindowUpdateLocker freeze(this);

    ChangeExpansion(node, true, true);
}

void NavigationPanel::OnCollapse(wxCommandEvent& /* event */)
{
    ASSERT(m_pMainFrame->getSelectedNode());
    Node* node = m_pMainFrame->getSelectedNode();
    if (!node)
    {
        return;  // This is theoretically impossible
    }

    const wxWindowUpdateLocker freeze(this);

    Node* parent = node->get_Parent();
    if (parent && parent->get_ChildCount())
    {
        for (const auto& child: parent->get_ChildNodePtrs())
        {
            ChangeExpansion(child.get(), false, false);
        }
    }
    else
    {
        ChangeExpansion(node, false, false);
    }
}

void NavigationPanel::OnCollExpand(wxCommandEvent& /* event */)
{
    ASSERT(m_pMainFrame->getSelectedNode());
    Node* node = m_pMainFrame->getSelectedNode();
    if (!node)
    {
        return;  // This is theoretically impossible
    }

    ExpandCollapse(node);
}

void NavigationPanel::ExpandCollapse(Node* node)
{
    const wxWindowUpdateLocker freeze(this);

    Node* parent = node->get_Parent();
    if (parent && parent->get_ChildCount())
    {
        for (const auto& child: parent->get_ChildNodePtrs())
        {
            if (child.get() != node)
            {
                ChangeExpansion(child.get(), false, false);
            }
        }
    }

    ChangeExpansion(node, true, true);
}
