/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <cstdint>  // std::int32_t

#include <wx/event.h>  // Event classes
#include <wx/menu.h>   // wxMenu and wxMenuBar classes

#include "gen_enums.h"  // Enumerations for generators

class Node;

// Creates a context-menu for the specified object
class NavPopupMenu : public wxMenu
{
public:
    NavPopupMenu(Node* node);

    // Call this from a generator if adding commands via PopupMenuAddCommands().
    //
    // The child parameter is the node that child sizers should be added to.
    void MenuAddChildSizerCommands(Node* child);

    enum class Menu : std::int32_t
    {
        Duplicate = START_NAV_CTX_MENU_IDS,

        MoveUp,
        MoveDown,
        MoveRight,
        MoveLeft,

        MergeSizerChildren,  // merge all children of a sizer into the parent sizer

        InsertWidget,

        NewParentBoxSizer,
        NewParentStaticSizer,
        NewParentWrapSizer,
        NewParentGridSizer,
        NewParentFlexGridSizer,
        NewParentGridbagSizer,

        NewParentFolder,

        NewChildBoxSizer,
        NewChildStaticSizer,
        NewChildWrapSizer,
        NewChildGridSizer,
        NewChildFlexGridSizer,
        NewChildGridbagSizer,
        NewChildStdDialgBtns,

        NewSiblingBoxSizer,
        NewSiblingStaticSizer,
        NewSiblingWrapSizer,
        NewSiblingGridSizer,
        NewSiblingFlexGridSizer,
        NewSiblingGridbagSizer,
        NewSiblingStdDialgBtns,

        ChangeToBoxSizer,
        ChangeToFlexGridSizer,
        ChangeToGridSizer,
        ChangeToStaticSizer,
        ChangeToWrapSizer,

        // This can only be used if there is just one possibility
        ChangeToNewNode,

        // 2-state wxCheckBox, 3-state wxCheckBox, wxRadioBox

        ChangeTo2StateCheckbox,
        ChangeTo3StateCheckbox,
        ChangeToRadioButton,

        // wxChoice, wxComboBox, wxListBox

        ChangeToChoiceBox,
        ChangeToComboBox,
        ChangeToListBox,

        // wxAuiNotebook, wxChoicebook
        ChangeToAuiBook,
        ChangeToChoiceBook,
        ChangeToListBook,
        ChangeToNoteBook,
        ChangeToSimpleBook,

        // REVIEW: [Randalphwa - 11-16-2022] Because the children are so different, I don't think
        // switching these is going to work.

        // ChangeTo_TOOL_BOOK,
        // ChangeTo_TREE_BOOK,

        AddImage,

        NewChildSpacer,
        NewSiblingSpacer,

        NewColumn,
        NewItem,

        NewNotebook,
        NewToolbar,
        NewInfobar,

        BordersAll,
        BordersNone,
        BordersHorizontal,
        BordersVertical,

        AddPage,
        AddRibbonPage,
        AddRibbonPanel,
        AddRibbonButtonbar,
        AddRibbonToolbar,
        AddRibbonGallery,
        AddRibbonButton,
        AddRibbonGalleryItem,

        AddPropgridPage,
        AddPropgridItem,
        AddPropgridCategory,

        AddMenu,
        AddMenuitem,
        AddSubmenu,
        AddMenuSeparator,

        AddTool,
        AddToolCombobox,
        AddToolDropdown,
        AddToolSlider,
        AddToolSpinctrl,
        AddToolStatictext,
        AddToolSeparator,

        // These are only used for wxToolBar

        AddToolStretchableSpace,

        // These are only used for wxAuiToolBar
        AddToolLabel,
        AddToolSpacer,
        AddToolStretchableSpacer,

        AddWizardPage,

        ProjectAddDialog,
        ProjectAddWindow,
        ProjectAddWizard,
        ProjectAddFolder,
        ProjectSortForms,

        ExpandAll,

        // Data List commands
        AddDataString,
        AddDataXml,

        // These are for Internal builds only
        SingleGenCpp,
        SingleGenPython,
        SingleGenRuby,
        SingleGenXRC,
        SingleGenPerl,
        CompareCode,

        TestingInfo,
        DebugKeyhh,
    };

protected:
    void AddToolbarCommands(Node* node);
    void OnMenuEvent(wxCommandEvent& event);
    void OnUpdateEvent(wxUpdateUIEvent& event);

    // Create menu for use when the current node is a sizer
    void CreateSizerMenu();

    // Creates menu for all non-sizer nodes
    void CreateCommonMenu();

    // Adds menu commands at the top of the menu. Calls node's generator to override the
    // default add commands as needed.
    void MenuAddCommands();

    // Cut, Copy, Paste, Delete, Duplicate
    void MenuAddStandardCommands();

    // This always adds up/down, and depending on the node, it also adds left/right and move
    // into new sizer.
    void MenuAddMoveCommands();

    void ChangeNode(GenEnum::GenName new_node_gen);
    void ChangeSizer(GenEnum::GenName new_sizer_gen);
    void CreateSizerParent(std::string_view widget);

    void AddSeparatorIfNeeded();

private:
    Node* m_parent { nullptr };  // parent of the node passed to ctor
    Node* m_node { nullptr };    // node passed to ctor
    Node* m_child { nullptr };
    Node* m_sizer_node { nullptr };  // node to add child sizers to
    GenEnum::GenName m_tool_name { GenEnum::GenName::gen_name_array_size };

    bool m_isPasteAllowed { true };
    bool m_is_parent_toolbar { false };  // true if parent is wxToolBar or wxAuiToolBar
};
