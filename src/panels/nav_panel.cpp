////////////////////////////////////////////////////////////////////////////
// Purpose:   Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

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
#include "navpopupmenu.h"     // NavPopupMenu -- Context-menu for an item
#include "navtoolbar_base.h"  // generated NavToolbar class
#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator class
#include "node_decl.h"        // NodeDeclaration class
#include "project_handler.h"  // ProjectHandler class
#include "propgrid_panel.h"   // PropGridPanel -- PropertyGrid class for node properties and events
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction
#include "utils.h"            // Utility functions that work with properties

constexpr size_t MaxLabelLength = 24;

// clang-format off
inline const std::vector<GenEnum::GenName> unsupported_gen_python = {

    gen_StaticCheckboxBoxSizer,
    gen_StaticRadioBtnBoxSizer,

    gen_wxContextMenuEvent,

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

NavigationPanel::NavigationPanel(wxWindow* parent, MainFrame* frame) : wxPanel(parent)
{
    m_pMainFrame = frame;

    SetWindowStyle(wxBORDER_RAISED);

    m_tree_ctrl = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                 wxTR_HAS_BUTTONS | wxTR_LINES_AT_ROOT | wxTR_DEFAULT_STYLE | wxBORDER_SUNKEN);
    int index = 0;
    m_iconList = new wxImageList(GenImageSize, GenImageSize);

    for (auto iter: NodeCreation.GetNodeDeclarationArray())
    {
        if (!iter)
        {
            // This will happen if there is an enumerated value but no generator for it
            continue;
        }
        m_iconList->Add(iter->GetImage());
        m_iconIdx[iter->gen_name()] = index++;
    }

    m_tree_ctrl->AssignImageList(m_iconList);

    auto toolbar = new NavToolbar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_NODIVIDER);

    toolbar->Realize();

    auto toolbar_sizer = new wxBoxSizer(wxHORIZONTAL);
    toolbar_sizer->AddSpacer(50);
    toolbar_sizer->Add(toolbar, wxSizerFlags().Expand().Border(wxBOTTOM | wxTOP));

    auto parent_sizer = new wxBoxSizer(wxVERTICAL);
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
             DeleteNode(event.GetNode());
         });

    Bind(wxEVT_MENU, &NavigationPanel::OnExpand, this, NavToolbar::id_NavExpand);
    Bind(wxEVT_MENU, &NavigationPanel::OnCollapse, this, NavToolbar::id_NavCollapse);
    Bind(wxEVT_MENU, &NavigationPanel::OnCollExpand, this, NavToolbar::id_NavCollExpand);

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

    Bind(wxEVT_UPDATE_UI, &NavigationPanel::OnUpdateEvent, this);

    m_pMainFrame->AddCustomEventHandler(GetEventHandler());
}

Node* NavigationPanel::GetNode(wxTreeItemId item)
{
    if (item.IsOk())
    {
        if (auto result = m_tree_node_map.find(item); result != m_tree_node_map.end())
        {
            return result->second;
        }
    }
    return nullptr;
}

void NavigationPanel::OnProjectUpdated()
{
    wxWindowUpdateLocker freeze(this);

#if defined(_DEBUG)
    if (wxGetApp().isFireCreationMsgs())
    {
        MSG_INFO("Navigation tree control recreated.");
    }
#endif  // _DEBUG

    m_tree_ctrl->DeleteAllItems();
    m_tree_node_map.clear();
    m_node_tree_map.clear();

    auto root = m_tree_ctrl->AddRoot(GetDisplayName(Project.ProjectNode()), GetImageIndex(Project.ProjectNode()), -1);
    m_node_tree_map[Project.ProjectNode()] = root;
    m_tree_node_map[root] = Project.ProjectNode();

    AddAllChildren(Project.ProjectNode());

    // First we expand everything, then we collapse all forms and folders
    ExpandAllNodes(Project.ProjectNode());

    std::vector<Node*> forms;
    Project.CollectForms(forms);

    for (const auto& form: forms)
    {
        if (auto result = m_node_tree_map.find(form); result != m_node_tree_map.end())
        {
            m_tree_ctrl->Collapse(result->second);
        }
    }

    for (const auto& folder: Project.ChildNodePtrs())
    {
        if (folder->isGen(gen_folder))
        {
            if (auto result = m_node_tree_map.find(folder.get()); result != m_node_tree_map.end())
            {
                m_tree_ctrl->Collapse(result->second);
            }
        }
    }
}

void NavigationPanel::OnSelChanged(wxTreeEvent& event)
{
    if (m_isSelChangeSuspended)
        return;

    auto id = event.GetItem();
    if (!id.IsOk())
        return;

    if (auto iter = m_tree_node_map.find(id); iter != m_tree_node_map.end())
    {
        // Selecting a node can result in multiple selection events getting fired as the
        // Mockup selects the current dialog, and the current book or page. In some cases a
        // generator will also fire off a selection event when mockup sets the current page
        // (e.g., wxEVT_NOTEBOOK_PAGE_CHANGED). There's no reason for the property grid to
        // update itself until we're done, so we lock it before the initial selection event.

        m_isSelChangeSuspended = true;
        m_pMainFrame->GetPropPanel()->Lock();
        m_pMainFrame->SelectNode(iter->second);
        m_pMainFrame->GetPropPanel()->UnLock();

        // It's possible for Mockup to select a page, so we need to be certain everything is
        // synced after the initial selection.

        if (iter->second != m_pMainFrame->GetSelectedNode())
            m_pMainFrame->SelectNode(iter->second);
        else
            m_pMainFrame->GetPropPanel()->Create();

        if (Project.value(prop_code_preference) == "Python")
        {
            if (std::find(unsupported_gen_python.begin(), unsupported_gen_python.end(), iter->second->gen_name()) !=
                unsupported_gen_python.end())
            {
                auto info = wxGetFrame().GetPropInfoBar();
                info->Dismiss();
                info->ShowMessage("This control is not supported by wxPython.", wxICON_INFORMATION);
            }
        }
        else if (Project.value(prop_code_preference) == "XRC")
        {
            if (std::find(unsupported_gen_XRC.begin(), unsupported_gen_XRC.end(), iter->second->gen_name()) !=
                unsupported_gen_XRC.end())
            {
                auto info = wxGetFrame().GetPropInfoBar();
                info->Dismiss();
                info->ShowMessage("This control is not supported by XRC.", wxICON_INFORMATION);
            }
        }

        m_isSelChangeSuspended = false;
    }
}

void NavigationPanel::OnRightClick(wxTreeEvent& event)
{
    auto id = event.GetItem();
    if (auto iter = m_tree_node_map.find(id); iter != m_tree_node_map.end())
    {
        auto menu = new NavPopupMenu(iter->second);
        auto pos = event.GetPoint();
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
    event.Allow();
}

void NavigationPanel::OnEndDrag(wxTreeEvent& event)
{
    auto itemSrc = m_drag_node;
    m_drag_node = nullptr;

    auto itemDst = event.GetItem();

    // ensure that itemDst is not itemSrc or a child of itemSrc
    auto item = itemDst;
    while (item.IsOk())
    {
        if (item == itemSrc)
        {
            if (wxMessageBox("Do you want to duplicate this item?", "Drop item onto itself", wxYES_NO) == wxYES)
            {
                wxGetFrame().DuplicateNode(GetNode(itemSrc));
                ExpandAllNodes(wxGetFrame().GetSelectedNode());
            }
            return;
        }
        item = m_tree_ctrl->GetItemParent(item);
    }

    auto node_src = GetNode(itemSrc);
    ASSERT(node_src);
    if (!node_src)
    {
        return;
    }

    auto node_dst = GetNode(itemDst);
    ASSERT(node_dst);
    if (!node_dst)
    {
        return;
    }

    if (node_dst->isGen(gen_wxSplitterWindow) && node_dst->GetChildCount() > 1)
    {
        wxMessageBox("A wxSplitterWindow can't have more than two windows.");
        return;
    }

    auto dst_parent = node_dst;
    while (!dst_parent->IsChildAllowed(node_src))
    {
        if (dst_parent->IsSizer())
        {
            wxMessageBox(tt_string() << "You can't drop a " << node_src->DeclName() << " onto a sizer.");
            return;
        }
        else if (dst_parent->IsContainer())
        {
            wxMessageBox(tt_string() << "You can't drop a " << node_src->DeclName() << " onto a " << dst_parent->DeclName()
                                     << '.');
            return;
        }
        else if (dst_parent->isGen(gen_Project))
        {
            wxMessageBox("Only forms can be dropped onto your project.");
            return;
        }
        dst_parent = dst_parent->GetParent();
        if (!dst_parent)
        {
            wxMessageBox(tt_string() << node_src->DeclName() << " can't be dropped onto this target.");
            return;
        }
    }

    if (dst_parent->isGen(gen_wxStdDialogButtonSizer))
    {
        wxMessageBox(tt_string() << "You can't drop a " << node_src->DeclName() << " onto a wxStdDialogBtnSizer.");
        return;
    }

    auto src_parent = node_src->GetParent();
    if (dst_parent->isGen(gen_wxGridBagSizer))
    {
        if (src_parent == dst_parent)
        {
            wxMessageBox("You cannot drag and drop an item within the same wxGridBagSizer. Use the Move commands instead.");
            return;
        }
    }
    else if (src_parent == dst_parent)
    {
        m_pMainFrame->PushUndoAction(
            std::make_shared<ChangePositionAction>(node_src, dst_parent->GetChildPosition(node_dst)));
        return;
    }

    m_pMainFrame->PushUndoAction(std::make_shared<ChangeParentAction>(node_src, dst_parent));
}

void NavigationPanel::OnNodeCreated(CustomEvent& event)
{
    wxWindowUpdateLocker freeze(this);
    InsertNode(event.GetNode());
}

void NavigationPanel::InsertNode(Node* node)
{
    auto node_parent = node->GetParent();
    ASSERT(node_parent);
    auto tree_parent = m_node_tree_map[node_parent];
    ASSERT(tree_parent);
    auto new_item = m_tree_ctrl->InsertItem(tree_parent, node_parent->GetChildPosition(node), GetDisplayName(node).wx_str(),
                                            GetImageIndex(node), -1);
    m_node_tree_map[node] = new_item;
    m_tree_node_map[new_item] = node;

    if (node->GetChildCount())
    {
        AddAllChildren(node);
        ExpandAllNodes(node);
    }
    else if (node->GetParent() && (node->GetParent()->isType(type_toolbar) || node->GetParent()->isType(type_aui_toolbar)))
    {
        // Insure that the toolbar is expanded when a new item is added to it
        ChangeExpansion(node->GetParent(), false, true);
    }
}

void NavigationPanel::AddAllChildren(Node* node_parent)
{
    auto tree_parent = m_node_tree_map[node_parent];
    ASSERT(tree_parent.IsOk());

    for (const auto& iter_child: node_parent->GetChildNodePtrs())
    {
        auto node = iter_child.get();
        auto new_item = m_tree_ctrl->AppendItem(tree_parent, GetDisplayName(node).wx_str(), GetImageIndex(node), -1);

        m_node_tree_map[node] = new_item;
        m_tree_node_map[new_item] = node;

        if (node->GetChildCount())
        {
            AddAllChildren(node);
        }
    }
}

int NavigationPanel::GetImageIndex(Node* node)
{
    auto name = node->gen_name();
    if (node->isGen(gen_wxBoxSizer))
    {
        if (node->isPropValue(prop_orientation, "wxVERTICAL"))
            name = gen_VerticalBoxSizer;
    }

    if (auto it = m_iconIdx.find(name); it != m_iconIdx.end())
        return it->second;
    else
        return 0;
}

void NavigationPanel::UpdateDisplayName(wxTreeItemId id, Node* node)
{
    m_tree_ctrl->SetItemText(id, GetDisplayName(node).wx_str());
}

tt_string NavigationPanel::GetDisplayName(Node* node) const
{
    tt_string display_name;
    if (node->HasValue(prop_label))
        display_name = node->prop_as_string(prop_label);
    else if (node->HasValue(prop_main_label))  // used by wxCommandLinkButton
        display_name = node->prop_as_string(prop_main_label);
    else if (node->HasValue(prop_var_name) && !node->isGen(gen_wxStaticBitmap))
        display_name = node->prop_as_string(prop_var_name);
    else if (node->HasValue(prop_class_name))
        display_name = node->prop_as_string(prop_class_name);
    else if (node->isGen(gen_ribbonTool))
        display_name = node->prop_as_string(prop_id);
    else if (node->isGen(gen_embedded_image))
    {
        tt_view_vector mstr(node->prop_as_string(prop_bitmap), ';');

        if (mstr.size() > IndexImage)
        {
            display_name = mstr[IndexImage].filename();
        }
    }
    else if (node->isGen(gen_wxStaticBitmap))
    {
        if (!node->HasValue(prop_bitmap))
        {
            if (node->HasValue(prop_var_name))
                display_name = node->prop_as_string(prop_var_name);
        }
        else
        {
            tt_view_vector mstr(node->prop_as_string(prop_bitmap), ';');
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
        display_name.Replace("&", "", true);

        if (!node->IsForm() && display_name.size() > MaxLabelLength)
        {
            display_name.erase(MaxLabelLength);
            display_name << "...";
        }
    }
    else
    {
        if (node->isGen(gen_Project))
            display_name << "Project: " << Project.ProjectFile().filename().utf8_string();
        else if (node->isGen(gen_wxContextMenuEvent))
        {
            display_name = node->prop_as_string(prop_handler_name);
            if (display_name.size() > MaxLabelLength)
            {
                display_name.erase(MaxLabelLength);
                display_name << "...";
            }
        }
        else
        {
            display_name << " (" << node->DeclName() << ")";
        }
    }

    return display_name;
}

void NavigationPanel::ExpandAllNodes(Node* node)
{
    wxWindowUpdateLocker freeze(this);
    if (auto item_it = m_node_tree_map.find(node); item_it != m_node_tree_map.end())
    {
        if (m_tree_ctrl->ItemHasChildren(item_it->second))
            m_tree_ctrl->Expand(item_it->second);
    }

    for (const auto& child: node->GetChildNodePtrs())
    {
        ExpandAllNodes(child.get());
    }
}

void NavigationPanel::DeleteNode(Node* node)
{
    wxWindowUpdateLocker freeze(this);
    EraseAllMaps(node);
}

void NavigationPanel::EraseAllMaps(Node* node)
{
    // If you delete a parent tree item it will automatically delete all children, but our maps won't reflect that. To keep
    // the treeview control and our maps in sync, we need to delete children before we delete the actual item.

    for (const auto& child: node->GetChildNodePtrs())
    {
        EraseAllMaps(child.get());
    }

    if (auto result = m_node_tree_map.find(node); result != m_node_tree_map.end())
    {
        m_tree_node_map.erase(result->second);
        if (result->second.IsOk())
            m_tree_ctrl->Delete(result->second);

        // Don't erase this until the iterator is no longer needed
        m_node_tree_map.erase(node);
    }
}

void NavigationPanel::OnNodeSelected(CustomEvent& event)
{
    auto node = event.GetNode();
    if (node->GetParent() && node->GetParent()->isGen(gen_wxGridBagSizer))
    {
        wxGetFrame().setStatusText(tt_string() << "Row: " << node->prop_as_int(prop_row)
                                               << ", Column: " << node->prop_as_int(prop_column));
    }
    else
    {
        if (node->HasValue(prop_var_name) && !node->prop_as_string(prop_class_access).starts_with("none"))
            wxGetFrame().setStatusText(node->prop_as_string(prop_var_name));
        else
            wxGetFrame().setStatusText(tt_empty_cstr);
    }

    if (m_isSelChangeSuspended)
        return;

    if (auto iter = m_node_tree_map.find(node); iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
    else
    {
        FAIL_MSG(tt_string("There is no tree item associated with this object.\n\tClass: ")
                 << node->DeclName() << "\n\tName: " << node->prop_as_string(prop_var_name).wx_str());
    }
}

void NavigationPanel::OnMultiPropChange(CustomEvent& event)
{
    auto& vector = static_cast<ModifyProperties*>(event.GetUndoCmd())->GetVector();
    for (auto& iter: vector)
    {
        CustomEvent prop_event(EVT_NodePropChange, iter.property);
        OnNodePropChange(prop_event);
    }
}

void NavigationPanel::OnNodePropChange(CustomEvent& event)
{
    auto prop = event.GetNodeProperty();

    if (prop->isProp(prop_var_name) || prop->isProp(prop_label) || prop->isProp(prop_class_name) ||
        prop->isProp(prop_bitmap))
    {
        auto class_name = prop->GetNode()->DeclName();
        if (class_name.contains("bookpage"))
        {
            if (auto it = m_node_tree_map.find(prop->GetNode()->GetChild(0)); it != m_node_tree_map.end())
            {
                UpdateDisplayName(it->second, it->first);
            }
        }
        else if (auto it = m_node_tree_map.find(prop->GetNode()); it != m_node_tree_map.end())
        {
            UpdateDisplayName(it->second, it->first);
        }
    }
    else if (prop->isProp(prop_id) && prop->GetNode()->isGen(gen_ribbonTool))
    {
        if (auto it = m_node_tree_map.find(prop->GetNode()); it != m_node_tree_map.end())
        {
            UpdateDisplayName(it->second, it->first);
        }
    }

    else if (prop->isProp(prop_orientation))
    {
        if (auto it = m_node_tree_map.find(prop->GetNode()); it != m_node_tree_map.end())
        {
            if (it->first->isGen(gen_VerticalBoxSizer) || it->first->isGen(gen_wxBoxSizer))
            {
                auto image_index = GetImageIndex(it->first);
                m_tree_ctrl->SetItemImage(it->second, image_index);
            }
        }
    }
    else if (prop->isProp(prop_handler_name))
    {
        if (auto it = m_node_tree_map.find(prop->GetNode()); it != m_node_tree_map.end())
        {
            UpdateDisplayName(it->second, it->first);
        }
    }
    else if (prop->isProp(prop_bitmap) && prop->GetNode()->isGen(gen_embedded_image))
    {
        if (auto it = m_node_tree_map.find(prop->GetNode()); it != m_node_tree_map.end())
        {
            UpdateDisplayName(it->second, it->first);
        }
    }
}

void NavigationPanel::OnUpdateEvent(wxUpdateUIEvent& event)
{
    auto node = m_pMainFrame->GetSelectedNode();
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
            event.Enable(node->GetChildCount() > 0);
            break;

        case NavToolbar::id_NavCollapse:
            event.Enable(node->GetParent() && node->GetParent()->GetChildCount() > 0);
            break;

        case NavToolbar::id_NavCollExpand:
            event.Enable((node->GetParent() && node->GetParent()->GetChildCount() > 0) || node->GetChildCount() > 0);
            break;
    }
}

void NavigationPanel::OnParentChange(CustomEvent& event)
{
    wxWindowUpdateLocker freeze(this);

    auto undo_cmd = static_cast<ChangeParentAction*>(event.GetUndoCmd());

    m_isSelChangeSuspended = true;
    m_tree_ctrl->Unselect();
    EraseAllMaps(undo_cmd->GetNode());
    InsertNode(undo_cmd->GetNode());
    m_isSelChangeSuspended = false;

    if (auto iter = m_node_tree_map.find(m_pMainFrame->GetSelectedNode()); iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
}

void NavigationPanel::OnPositionChange(CustomEvent& event)
{
    wxWindowUpdateLocker freeze(this);

    auto undo_cmd = static_cast<ChangePositionAction*>(event.GetUndoCmd());

    m_isSelChangeSuspended = true;
    m_tree_ctrl->Unselect();
    EraseAllMaps(undo_cmd->GetNode());
    InsertNode(undo_cmd->GetNode());
    m_isSelChangeSuspended = false;

    if (auto iter = m_node_tree_map.find(m_pMainFrame->GetSelectedNode()); iter != m_node_tree_map.end())
    {
        m_tree_ctrl->EnsureVisible(iter->second);
        m_tree_ctrl->SelectItem(iter->second);
    }
}

void NavigationPanel::ChangeExpansion(Node* node, bool include_children, bool expand)
{
    if (include_children)
    {
        for (const auto& child: node->GetChildNodePtrs())
        {
            if (child->GetChildCount())
            {
                ChangeExpansion(child.get(), include_children, expand);
            }
        }
    }
    if (node->GetChildCount())
    {
        if (expand)
        {
            if (auto item = m_node_tree_map.find(node); item != m_node_tree_map.end())
            {
                m_tree_ctrl->Expand(item->second);
            }
        }
        else
        {
            if (auto item = m_node_tree_map.find(node); item != m_node_tree_map.end())
            {
                m_tree_ctrl->Collapse(item->second);
            }
        }
    }
}

void NavigationPanel::OnExpand(wxCommandEvent&)
{
    ASSERT(m_pMainFrame->GetSelectedNode());
    auto node = m_pMainFrame->GetSelectedNode();
    if (!node)
        return;  // This is theoretically impossible

    wxWindowUpdateLocker freeze(this);

    ChangeExpansion(node, true, true);
}

void NavigationPanel::OnCollapse(wxCommandEvent&)
{
    ASSERT(m_pMainFrame->GetSelectedNode());
    auto node = m_pMainFrame->GetSelectedNode();
    if (!node)
        return;  // This is theoretically impossible

    wxWindowUpdateLocker freeze(this);

    auto parent = node->GetParent();
    if (parent && parent->GetChildCount())
    {
        for (const auto& child: parent->GetChildNodePtrs())
        {
            ChangeExpansion(child.get(), false, false);
        }
    }
    else
    {
        ChangeExpansion(node, false, false);
    }
}

void NavigationPanel::OnCollExpand(wxCommandEvent&)
{
    ASSERT(m_pMainFrame->GetSelectedNode());
    auto node = m_pMainFrame->GetSelectedNode();
    if (!node)
        return;  // This is theoretically impossible

    ExpandCollapse(node);
}

void NavigationPanel::ExpandCollapse(Node* node)
{
    wxWindowUpdateLocker freeze(this);

    auto parent = node->GetParent();
    if (parent && parent->GetChildCount())
    {
        for (const auto& child: parent->GetChildNodePtrs())
        {
            if (child.get() != node)
            {
                ChangeExpansion(child.get(), false, false);
            }
        }
    }

    ChangeExpansion(node, true, true);
}
