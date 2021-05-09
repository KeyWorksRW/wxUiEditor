/////////////////////////////////////////////////////////////////////////////
// Purpose:   Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

#include <wx/event.h>     // Event classes
#include <wx/menu.h>      // wxMenu and wxMenuBar classes
#include <wx/panel.h>     // Base header for wxPanel
#include <wx/treectrl.h>  // wxTreeCtrl base header
#include <wx/window.h>    // wxWindowBase class - the interface of wxWindow

#include "gen_enums.h"     // Enumerations for generators
#include "node_classes.h"  // Forward defintions of Node classes

using namespace GenEnum;

class CustomEvent;
class MainFrame;

class NavigationPanel : public wxPanel
{
public:
    NavigationPanel(wxWindow* parent, MainFrame* frame);
    void ChangeExpansion(Node* node, bool include_children, bool expand);

protected:
    void AddAllNodes();
    void AddChildNodes(Node* child, wxTreeItemId& parent, bool is_root = false);
    void AddNode(Node* item, Node* parent);
    void DeleteNode(Node* item);
    void EraseAllMaps(Node* node);
    void ExpandAllNodes(Node* node);
    int GetImageIndex(Node* node);
    Node* GetNode(wxTreeItemId item);
    void UpdateDisplayName(wxTreeItemId id, Node* node);

    // Event handlers

    void OnBeginDrag(wxTreeEvent& event);
    void OnEndDrag(wxTreeEvent& event);
    void OnRightClick(wxTreeEvent& event);
    void OnSelChanged(wxTreeEvent& event);

    void OnUpdateEvent(wxUpdateUIEvent& event);

    void OnCollExpand(wxCommandEvent& event);
    void OnCollapse(wxCommandEvent& event);
    void OnExpand(wxCommandEvent& event);

    void OnNodeSelected(CustomEvent& event);
    void OnNodePropChange(CustomEvent& event);

private:
    MainFrame* m_pMainFrame;

    // wxTreeItemId is a class, so you can't use it as a key in a std::unordered map because you can't use the built-in
    // hash algorithm. We don't need to sort the Node pointers using std::map but it avoids the overhead of hashing the
    // pointer, so a std::unordered_map isn't likely to be significantly faster.

    std::map<Node*, wxTreeItemId> m_node_tree_map;
    std::map<wxTreeItemId, Node*> m_tree_node_map;

    wxImageList* m_iconList;

    std::map<GenName, int> m_iconIdx;

    wxTreeCtrl* m_tree_ctrl;

    wxTreeItemId m_drag_node;

    bool m_isSelChangeSuspended { false };
};
