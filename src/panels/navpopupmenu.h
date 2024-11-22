/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/event.h>  // Event classes
#include <wx/menu.h>   // wxMenu and wxMenuBar classes

#include "gen_enums.h"     // Enumerations for generators
#include "node_classes.h"  // Forward defintions of Node classes

// Creates a context-menu for the specified object
class NavPopupMenu : public wxMenu
{
public:
    NavPopupMenu(Node* node);

    // Call this from a generator if adding commands via PopupMenuAddCommands().
    //
    // The child parameter is the node that child sizers should be added to.
    void MenuAddChildSizerCommands(Node* child);

    enum
    {
        MenuDUPLICATE = wxID_HIGHEST + 2000,

        MenuMOVE_UP,
        MenuMOVE_DOWN,
        MenuMOVE_RIGHT,
        MenuMOVE_LEFT,

        MenuInsertWidget,

        MenuNEW_PARENT_BOX_SIZER,
        MenuNEW_PARENT_STATIC_SIZER,
        MenuNEW_PARENT_WRAP_SIZER,
        MenuNEW_PARENT_GRID_SIZER,
        MenuNEW_PARENT_FLEX_GRID_SIZER,
        MenuNEW_PARENT_GRIDBAG_SIZER,

        MenuNEW_PARENT_FOLDER,

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

        MenuChangeTo_FLEX_GRID_SIZER,
        MenuChangeTo_GRID_SIZER,
        MenuChangeTo_STATIC_SIZER,
        MenuChangeTo_WRAP_SIZER,

        // This can only be used if there is just one possibility
        MenuChangeTo_NEW_NODE,

        // 2-state wxCheckBox, 3-state wxCheckBox, wxRadioBox

        MenuChangeTo_2STATE_CHECKBOX,
        MenuChangeTo_3STATE_CHECKBOX,
        MenuChangeTo_RADIO_BUTTON,

        // wxChoice, wxComboBox, wxListBox

        MenuChangeTo_CHOICE_BOX,
        MenuChangeTo_COMBO_BOX,
        MenuChangeTo_LIST_BOX,

        // wxAuiNotebook, wxChoicebook
        MenuChangeTo_AUI_BOOK,
        MenuChangeTo_CHOICE_BOOK,
        MenuChangeTo_LIST_BOOK,
        MenuChangeTo_NOTE_BOOK,
        MenuChangeTo_SIMPLE_BOOK,

        // REVIEW: [Randalphwa - 11-16-2022] Because the children are so different, I don't think
        // switching these is going to work.

        // MenuChangeTo_TOOL_BOOK,
        // MenuChangeTo_TREE_BOOK,

        MenuADD_IMAGE,

        MenuNEW_CHILD_SPACER,
        MenuNEW_SIBLING_SPACER,

        MenuNEW_COLUMN,
        MenuNEW_ITEM,

        MenuNEW_NOTEBOOK,
        MenuNEW_TOOLBAR,
        MenuNEW_INFOBAR,

        MenuBORDERS_ALL,
        MenuBORDERS_NONE,
        MenuBORDERS_HORIZONTAL,
        MenuBORDERS_VERTICAL,

        MenuADD_PAGE,
        MenuADD_RIBBON_PAGE,
        MenuADD_RIBBON_PANEL,
        MenuADD_RIBBON_BUTTONBAR,
        MenuADD_RIBBON_TOOLBAR,
        MenuADD_RIBBON_GALLERY,
        MenuADD_RIBBON_BUTTON,
        MenuADD_RIBBON_GALLERY_ITEM,

        MenuADD_PROPGRID_PAGE,
        MenuADD_PROPGRID_ITEM,
        MenuADD_PROPGRID_CATEGORY,

        MenuADD_MENU,
        MenuADD_MENUITEM,
        MenuADD_SUBMENU,
        MenuADD_MENU_SEPARATOR,

        MenuADD_TOOL,
        MenuADD_TOOL_COMBOBOX,
        MenuADD_TOOL_DROPDOWN,
        MenuADD_TOOL_SLIDER,
        MenuADD_TOOL_SPINCTRL,
        MenuADD_TOOL_STATICTEXT,
        MenuADD_TOOL_SEPARATOR,

        // These are only used for wxToolBar

        MenuADD_TOOL_STRETCHABLE_SPACE,

        // These are only used for wxAuiToolBar
        MenuADD_TOOL_LABEL,
        MenuADD_TOOL_SPACER,
        MenuADD_TOOL_STRETCHABLE_SPACER,

        MenuADD_WIZARD_PAGE,

        MenuPROJECT_ADD_DIALOG,
        MenuPROJECT_ADD_WINDOW,
        MenuPROJECT_ADD_WIZARD,
        MenuPROJECT_ADD_FOLDER,
        MenuPROJECT_SORT_FORMS,

        MenuEXPAND_ALL,

        // Data List commands
        MenuADD_DATA_STRING,
        MenuADD_DATA_XML,

        // These are for Internal builds only
        MenuSingleGenCpp,
        MenuSingleGenPython,
        MenuSingleGenRuby,
        MenuSingleGenXRC,
        MenuSingleGenFortran,
        MenuSingleGenHaskell,
        MenuSingleGenLua,
        MenuSingleGenPerl,
        MenuSingleGenRust,

        MenuTESTING_INFO,
        MenuDEBUG_KEYHH,
    };

protected:
    void AddToolbarCommands(Node* node);
    void OnMenuEvent(wxCommandEvent& event);
    void OnUpdateEvent(wxUpdateUIEvent& event);

    // Create menu for use when the current node is a sizer
    void CreateSizerMenu(Node* node);

    // Creates menu for all non-sizer nodes
    void CreateCommonMenu(Node* node);

    // Adds menu commands at the top of the menu. Calls node's generator to override the
    // default add commands as needed.
    void MenuAddCommands(Node* node);

    // Cut, Copy, Paste, Delete, Duplicate
    void MenuAddStandardCommands(Node* node);

    // This always adds up/down, and depending on the node, it also adds left/right and move
    // into new sizer.
    void MenuAddMoveCommands(Node* node);

    void ChangeNode(GenEnum::GenName new_node_gen);
    void ChangeSizer(GenEnum::GenName new_sizer_gen);
    void CreateSizerParent(Node* node, tt_string_view widget);

    void AddSeparatorIfNeeded();

private:
    Node* m_node { nullptr };
    Node* m_child { nullptr };
    Node* m_sizer_node { nullptr };  // node to add child sizers to
    GenEnum::GenName m_tool_name { GenEnum::GenName::gen_name_array_size };

    bool m_isPasteAllowed { true };
    bool m_is_parent_toolbar { false };  // true if parent is wxToolBar or wxAuiToolBar
};
