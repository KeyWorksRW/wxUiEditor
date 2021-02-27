/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/event.h>  // Event classes
#include <wx/menu.h>   // wxMenu and wxMenuBar classes

#include "node_classes.h"  // Forward defintions of Node classes

// Creates a context-menu for the specified object
class NavPopupMenu : public wxMenu
{
public:
    NavPopupMenu(Node* node);

protected:
    void OnAddNew(wxCommandEvent& event);
    void OnBorders(wxCommandEvent& event);
    void OnMenuEvent(wxCommandEvent& event);
    void OnUpdateEvent(wxUpdateUIEvent& event);

    void CreateBarMenu(Node* node);
    void CreateBookMenu(Node* node);
    void CreateContainerMenu(Node* node);
    void CreateMenuMenu(Node* node);
    void CreateProjectMenu(Node* node);
    void CreateTopSizerMenu(Node* node);
    void CreateWizardMenu(Node* node);

    void CreateSizerParent(Node* node, ttlib::cview widget);

    enum
    {
        MenuCUT = wxID_HIGHEST + 2000,
        MenuCOPY,
        MenuPASTE,
        MenuDUPLICATE,
        MenuDELETE,

        MenuMOVE_UP,
        MenuMOVE_DOWN,
        MenuMOVE_RIGHT,
        MenuMOVE_LEFT,

        MenuNEW_PARENT_BOX_SIZER,
        MenuNEW_PARENT_STATIC_SIZER,
        MenuNEW_PARENT_WRAP_SIZER,
        MenuNEW_PARENT_GRID_SIZER,
        MenuNEW_PARENT_FLEX_GRID_SIZER,
        MenuNEW_PARENT_GRIDBAG_SIZER,

        MenuNEW_CHILD_BOX_SIZER,
        MenuNEW_CHILD_STATIC_SIZER,
        MenuNEW_CHILD_WRAP_SIZER,
        MenuNEW_CHILD_GRID_SIZER,
        MenuNEW_CHILD_FLEX_GRID_SIZER,
        MenuNEW_CHILD_GRIDBAG_SIZER,
        MenuNEW_CHILD_STD_DIALG_BTNS,

        MenuNEW_SIBLING_BOX_SIZER,
        MenuNEW_SIBLING_STATIC_SIZER,
        MenuNEW_SIBLING_WRAP_SIZER,
        MenuNEW_SIBLING_GRID_SIZER,
        MenuNEW_SIBLING_FLEX_GRID_SIZER,
        MenuNEW_SIBLING_GRIDBAG_SIZER,
        MenuNEW_SIBLING_STD_DIALG_BTNS,

        MenuNEW_CHILD_SPACER,
        MenuNEW_SIBLING_SPACER,

        MenuNEW_ITEM,

        MenuNEW_NOTEBOOK,
        MenuNEW_TOOLBAR,
        MenuNEW_INFOBAR,

        MenuRESET_ID,
        MenuRESET_SIZE,
        MenuRESET_MIN_SIZE,
        MenuRESET_MAX_SIZE,

        MenuBORDERS_ALL,
        MenuBORDERS_NONE,
        MenuBORDERS_HORIZONTAL,
        MenuBORDERS_VERTICAL,

        MenuADD_PAGE,

        MenuADD_MENU,
        MenuADD_MENUITEM,
        MenuADD_SUBMENU,
        MenuADD_SEPARATOR,

        MenuADD_WIZARD_PAGE,

        MenuPROJECT_ADD_DIALOG,
        MenuPROJECT_ADD_TABBED_DIALOG,
        MenuPROJECT_ADD_WINDOW,
        MenuPROJECT_ADD_WIZARD,

        MenuTESTING_INFO,
        MenuDEBUG_KEYHH,
    };

private:
    Node* m_node;
    Node* m_child { nullptr };
    ttlib::cstr m_tool_name;  // used by MenuNEW_ITEM -> wxGetApp().CreateToolNode(m_tool_name);
};
