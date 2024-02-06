/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>   // wxArtProvider class
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "navpopupmenu.h"  // NavPopupMenu

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "bitmaps.h"         // Contains various images handling functions
#include "clipboard.h"       // Handles reading and writing OS clipboard data
#include "mainframe.h"       // MainFrame -- Main window frame
#include "nav_panel.h"       // NavigationPanel -- Navigation Panel
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "undo_cmds.h"       // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"           // Utility functions that work with properties

#include "../newdialogs/new_dialog.h"  // NewDialog -- Dialog for creating a new project dialog
#include "../newdialogs/new_frame.h"   // NewFrame -- Dialog for creating a new project wxFrame
#include "../newdialogs/new_wizard.h"  // NewWizard -- Dialog for creating a new wizard

NavPopupMenu::NavPopupMenu(Node* node) : m_node(node)
{
    if (!node)
    {
        ASSERT(node);
        return;  // theoretically impossible, but don't crash if it happens
    }

    if (node->getParent() && node->getParent()->isToolBar())
    {
        m_is_parent_toolbar = true;
    }

    if (node->isSizer())
    {
        CreateSizerMenu(node);
    }
    else
    {
        CreateCommonMenu(node);
    }
}

void NavPopupMenu::OnMenuEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MenuNEW_ITEM:
            if (m_tool_name < gen_name_array_size)
            {
                if (m_node->isType(type_bookpage) || m_node->isType(type_wizardpagesimple))
                {
                    if (m_child && m_child->isSizer())
                    {
                        m_child = m_child->getParent();
                    }
                }

                if (m_child)
                    m_child->createToolNode(m_tool_name);
                else
                    wxGetFrame().createToolNode(m_tool_name);
            }
            break;

        case MenuNEW_COLUMN:
            if (m_tool_name == gen_wxTreeListCtrl)
            {
                if (m_child)
                    m_child->createToolNode(gen_TreeListCtrlColumn);
                else
                    wxGetFrame().createToolNode(gen_TreeListCtrlColumn);
            }
            break;

        case MenuNEW_SIBLING_BOX_SIZER:
            m_node->getParent()->createToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_SIBLING_STATIC_SIZER:
            m_node->getParent()->createToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_SIBLING_WRAP_SIZER:
            m_node->getParent()->createToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_SIBLING_GRID_SIZER:
            m_node->getParent()->createToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_SIBLING_FLEX_GRID_SIZER:
            m_node->getParent()->createToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_SIBLING_GRIDBAG_SIZER:
            m_node->getParent()->createToolNode(gen_wxGridBagSizer);
            break;

        case MenuNEW_SIBLING_STD_DIALG_BTNS:
            m_node->getParent()->createToolNode(gen_wxStdDialogButtonSizer);
            break;

        case MenuNEW_SIBLING_SPACER:
            m_node->getParent()->createToolNode(gen_spacer);
            break;

        case MenuNEW_CHILD_BOX_SIZER:
            m_sizer_node->createToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_CHILD_STATIC_SIZER:
            m_sizer_node->createToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_CHILD_WRAP_SIZER:
            m_sizer_node->createToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_CHILD_GRID_SIZER:
            m_sizer_node->createToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_CHILD_FLEX_GRID_SIZER:
            m_sizer_node->createToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_CHILD_GRIDBAG_SIZER:
            m_sizer_node->createToolNode(gen_wxGridBagSizer);
            break;

        case MenuNEW_CHILD_STD_DIALG_BTNS:
            wxGetFrame().createToolNode(gen_wxStdDialogButtonSizer);
            break;

        case MenuNEW_CHILD_SPACER:
            wxGetFrame().createToolNode(gen_spacer);
            break;

        case MenuNEW_TOOLBAR:
            wxGetFrame().createToolNode(gen_wxToolBar);
            break;

        case MenuNEW_INFOBAR:
            wxGetFrame().createToolNode(gen_wxInfoBar);
            break;

        case MenuADD_MENU:
            wxGetFrame().createToolNode(gen_wxMenu);
            break;

        case MenuADD_TOOL_SEPARATOR:
            if (m_child)
                m_child->createToolNode(gen_toolSeparator);
            else
                wxGetFrame().createToolNode(gen_toolSeparator);
            break;

        case MenuEXPAND_ALL:
            wxGetFrame().getNavigationPanel()->ExpandCollapse(m_node);
            wxGetFrame().SelectNode(m_node);
            break;

        case wxID_CUT:
            wxGetFrame().CutNode(m_node);
            break;

        case wxID_COPY:
            wxGetFrame().CopyNode(m_node);
            break;

        case wxID_PASTE:
            wxGetFrame().PasteNode(m_node);
            break;

        case MenuDUPLICATE:
            wxGetFrame().DuplicateNode(m_node);
            break;

        case wxID_DELETE:
            wxGetFrame().DeleteNode(m_node);
            break;

        case MenuMOVE_UP:
            wxGetFrame().MoveNode(m_node, MoveDirection::Up);
            break;

        case MenuMOVE_DOWN:
            wxGetFrame().MoveNode(m_node, MoveDirection::Down);
            break;

        case MenuMOVE_RIGHT:
            wxGetFrame().MoveNode(m_node, MoveDirection::Right);
            break;

        case MenuMOVE_LEFT:
            wxGetFrame().MoveNode(m_node, MoveDirection::Left);
            break;

        case MenuNEW_PARENT_BOX_SIZER:
            CreateSizerParent(m_node, "wxBoxSizer");
            break;

        case MenuNEW_PARENT_STATIC_SIZER:
            CreateSizerParent(m_node, "wxStaticBoxSizer");
            break;

        case MenuNEW_PARENT_WRAP_SIZER:
            CreateSizerParent(m_node, "wxWrapSizer");
            break;

        case MenuNEW_PARENT_GRID_SIZER:
            CreateSizerParent(m_node, "wxGridSizer");
            break;

        case MenuNEW_PARENT_FLEX_GRID_SIZER:
            CreateSizerParent(m_node, "wxFlexGridSizer");
            break;

        case MenuNEW_PARENT_GRIDBAG_SIZER:
            CreateSizerParent(m_node, "wxGridBagSizer");
            break;

        case MenuNEW_PARENT_FOLDER:
            CreateSizerParent(m_node, "folder");
            break;

        case MenuChangeTo_FLEX_GRID_SIZER:
            ChangeSizer(gen_wxFlexGridSizer);
            break;

        case MenuChangeTo_AUI_BOOK:
            ChangeNode(gen_wxAuiNotebook);
            break;

        case MenuChangeTo_CHOICE_BOOK:
            ChangeNode(gen_wxChoicebook);
            break;

        case MenuChangeTo_LIST_BOOK:
            ChangeNode(gen_wxListbook);
            break;

        case MenuChangeTo_NOTE_BOOK:
            ChangeNode(gen_wxNotebook);
            break;

        case MenuChangeTo_SIMPLE_BOOK:
            ChangeNode(gen_wxSimplebook);
            break;

        case MenuChangeTo_2STATE_CHECKBOX:
            ChangeNode(gen_wxCheckBox);
            break;

        case MenuChangeTo_3STATE_CHECKBOX:
            ChangeNode(gen_Check3State);
            break;

        case MenuChangeTo_RADIO_BUTTON:
            ChangeNode(gen_wxRadioButton);
            break;

        case MenuChangeTo_CHOICE_BOX:
            ChangeNode(gen_wxChoice);
            break;

        case MenuChangeTo_COMBO_BOX:
            ChangeNode(gen_wxComboBox);
            break;

        case MenuChangeTo_LIST_BOX:
            ChangeNode(gen_wxListBox);
            break;

        case MenuChangeTo_GRID_SIZER:
            ChangeSizer(gen_wxGridSizer);
            break;

        case MenuChangeTo_STATIC_SIZER:
            ChangeSizer(gen_wxStaticBoxSizer);
            break;

        case MenuChangeTo_WRAP_SIZER:
            ChangeSizer(gen_wxWrapSizer);
            break;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
        case MenuSingleGenCpp:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleCpp(dummy);
            }
            break;

        case MenuSingleGenPython:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSinglePython(dummy);
            }
            break;

        case MenuSingleGenRuby:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleRuby(dummy);
            }
            break;

        case MenuSingleGenRust:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleRust(dummy);
            }
            break;

        case MenuSingleGenXRC:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleXRC(dummy);
            }
            break;
#endif

        case MenuADD_PAGE:
            if (m_node->isGen(gen_BookPage))
            {
                m_node->getParent()->createToolNode(gen_BookPage);
            }
            else if (m_node->isGen(gen_wxWizardPageSimple))
            {
                m_node->getParent()->createToolNode(gen_wxWizardPageSimple);
            }
            else
            {
                wxGetFrame().createToolNode(gen_wxPanel);
            }
            break;

        default:
            break;
    }
}

void NavPopupMenu::OnUpdateEvent(wxUpdateUIEvent& event)
{
    switch (event.GetId())
    {
        case wxID_CUT:
        case wxID_COPY:
            if (m_node && !m_node->isGen(gen_Project))
                event.Enable(true);
            else
                event.Enable(wxGetFrame().CanCopyNode());
            break;

        case MenuMOVE_UP:
            Enable(MenuMOVE_UP, wxGetFrame().MoveNode(m_node, MoveDirection::Up, true));
            break;

        case MenuMOVE_DOWN:
            Enable(MenuMOVE_DOWN, wxGetFrame().MoveNode(m_node, MoveDirection::Down, true));
            break;

        case MenuMOVE_LEFT:
            Enable(MenuMOVE_LEFT, wxGetFrame().MoveNode(m_node, MoveDirection::Left, true));
            break;

        case MenuMOVE_RIGHT:
            Enable(MenuMOVE_RIGHT, wxGetFrame().MoveNode(m_node, MoveDirection::Right, true));
            break;
    }
}

void NavPopupMenu::CreateSizerMenu(Node* node)
{
    // This needs to be added first to cover all menu ids that aren't specically bound to an id.
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    m_sizer_node = node;
    auto dpi_size = wxGetFrame().FromDIP(wxSize(16, 16));

    bool isTopSizer = (node->getParent()->isForm() || node->getParent()->isContainer());
    wxMenuItem* menu_item;
    wxMenu* sub_menu;

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

    AppendSubMenu(sub_menu, "Add child sizer");

    if (!isTopSizer)
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));
        AppendSubMenu(sub_menu, "Add sibling sizer");

        AppendSeparator();
        MenuAddMoveCommands(node);
    }

    if (node->isGen(gen_wxBoxSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    else if (node->isGen(gen_wxGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    else if (node->isGen(gen_wxFlexGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    else if (node->isGen(gen_wxWrapSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }

    AppendSeparator();
    MenuAddStandardCommands(node);
}

void NavPopupMenu::CreateCommonMenu(Node* node)
{
    // This needs to be added first to cover all menu ids that aren't specically bound to an id.
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    MenuAddCommands(node);
    MenuAddMoveCommands(node);
    if (!node->isGen(gen_Images) && !node->isGen(gen_embedded_image) && !node->isGen(gen_Data) &&
        !node->isGen(gen_data_string))
    {
        AppendSeparator();
    }
    MenuAddStandardCommands(node);
}

void NavPopupMenu::MenuAddCommands(Node* node)
{
#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    if (node->isForm())
    {
        Append(MenuSingleGenCpp, "Generate C++ for this form");
        Append(MenuSingleGenPython, "Generate Python for this form");
        Append(MenuSingleGenRuby, "Generate Ruby for this form");
        Append(MenuSingleGenRust, "Generate Rust for this form");
        Append(MenuSingleGenXRC, "Generate XRC for this form");
        AppendSeparator();
    }
#endif

    if (node->isForm() || node->isGen(gen_Images) || node->isGen(gen_embedded_image) || node->isGen(gen_Data) ||
        node->isGen(gen_data_string))
    {
        if (node->isGen(gen_wxPropertySheetDialog))
        {
            Append(MenuADD_PAGE, "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_BookPage);
                },
                MenuADD_PAGE);
            return;
        }
        if (!node->isGen(gen_wxWizard) && !node->isToolBar())
        {
            return;
        }
    }

    if (node->isGen(gen_wxStatusBar) || node->isGen(gen_embedded_image))
    {
        return;
    }

    if (auto gen = node->getGenerator(); gen)
    {
        if (gen->PopupMenuAddCommands(this, node))
        {
            AppendSeparator();
            return;
        }
    }

    bool add_sizer = true;

    switch (node->getGenName())
    {
        case gen_wxAuiNotebook:
        case gen_wxChoicebook:
        case gen_wxListbook:
        case gen_wxNotebook:
        case gen_wxSimplebook:
        case gen_wxToolbook:
        case gen_wxTreebook:
            add_sizer = false;
            Append(MenuADD_PAGE, "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_BookPage);
                },
                MenuADD_PAGE);
            break;

        case gen_BookPage:
            add_sizer = true;
            Append(MenuADD_PAGE, "Add Page");
            break;

        case gen_wxRibbonBar:
        case gen_RibbonBar:
            add_sizer = false;
            Append(MenuADD_RIBBON_PAGE, "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_wxRibbonPage);
                },
                MenuADD_RIBBON_PAGE);
            break;

        case gen_wxRibbonPage:
            add_sizer = false;
            Append(MenuADD_RIBBON_PANEL, "Add Panel");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_wxRibbonPanel);
                },
                MenuADD_RIBBON_PANEL);
            break;

        case gen_wxRibbonPanel:
            add_sizer = false;
            if (node->getChildCount())
            {
                Append(MenuADD_RIBBON_PANEL, "Add Panel");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().createToolNode(gen_wxRibbonPanel);
                    },
                    MenuADD_RIBBON_PANEL);
                break;
            }
            else
            {
                Append(MenuADD_RIBBON_BUTTONBAR, "Add Button Bar");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().createToolNode(gen_wxRibbonButtonBar);
                    },
                    MenuADD_RIBBON_BUTTONBAR);

                Append(MenuADD_RIBBON_TOOLBAR, "Add Tool Bar");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().createToolNode(gen_wxRibbonToolBar);
                    },
                    MenuADD_RIBBON_TOOLBAR);

                Append(MenuADD_RIBBON_GALLERY, "Add Gallery");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().createToolNode(gen_wxRibbonGallery);
                    },
                    MenuADD_RIBBON_GALLERY);
            }
            break;

        case gen_wxRibbonButtonBar:
        case gen_wxRibbonToolBar:
        case gen_ribbonButton:
        case gen_ribbonTool:
            add_sizer = false;
            Append(MenuADD_RIBBON_BUTTON, "Add Button");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_ribbonButton);
                },
                MenuADD_RIBBON_BUTTON);
            break;

        case gen_wxRibbonGallery:
        case gen_ribbonGalleryItem:
            add_sizer = false;
            Append(MenuADD_RIBBON_GALLERY_ITEM, "Add Gallery Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_ribbonGalleryItem);
                },
                MenuADD_RIBBON_GALLERY_ITEM);
            break;

        case gen_AuiToolBar:
        case gen_wxAuiToolBar:
        case gen_auitool:
            add_sizer = false;
            AddToolbarCommands(node);
            break;

        case gen_wxToolBar:
        case gen_ToolBar:
        case gen_tool:
        case gen_toolSeparator:
        case gen_toolStretchable:
            add_sizer = false;
            AddToolbarCommands(node);
            break;

        case gen_wxMenuBar:
        case gen_MenuBar:
            add_sizer = false;
            Append(MenuADD_MENU, "Add Menu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_wxMenu);
                },
                MenuADD_MENU);
            break;

        case gen_wxMenu:
        case gen_wxMenuItem:
        case gen_submenu:
        case gen_separator:
        case gen_tool_dropdown:
            add_sizer = false;
            Append(MenuADD_MENUITEM, "Add Menu Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_wxMenuItem);
                },
                MenuADD_MENUITEM);

            Append(MenuADD_SUBMENU, "Add Submenu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_submenu);
                },
                MenuADD_SUBMENU);

            Append(MenuADD_MENU_SEPARATOR, "Add Separator");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_separator);
                },
                MenuADD_MENU_SEPARATOR);

            break;

        case gen_wxPropertyGridManager:
            add_sizer = false;
            Append(MenuADD_PROPGRID_PAGE, "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_propGridPage);
                },
                MenuADD_PROPGRID_PAGE);
            break;

        case gen_wxPropertyGrid:
        case gen_propGridPage:
        case gen_propGridCategory:
        case gen_propGridItem:
            add_sizer = false;
            if (!node->isGen(gen_propGridCategory) && !node->isGen(gen_propGridItem))
            {
                Append(MenuADD_PROPGRID_CATEGORY, "Add Category");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().createToolNode(gen_propGridCategory);
                    },
                    MenuADD_PROPGRID_CATEGORY);
            }
            Append(MenuADD_PROPGRID_ITEM, "Add Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().createToolNode(gen_propGridItem);
                },
                MenuADD_PROPGRID_ITEM);
            break;

        default:
            if (m_is_parent_toolbar)
            {
                add_sizer = false;
                AddToolbarCommands(node);
            }
            else
            {
                Append(MenuNEW_CHILD_SPACER, "Add spacer");
            }
            break;
    }

    if (add_sizer)
    {
        MenuAddChildSizerCommands(node);
    }

    if (!node->isGen(gen_Project))
    {
        AppendSeparator();
    }
}

void NavPopupMenu::MenuAddChildSizerCommands(Node* child)
{
    m_sizer_node = child;
    auto dpi_size = wxGetFrame().FromDIP(wxSize(16, 16));

    auto sub_menu = new wxMenu;
    auto menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

    if (child->isParent(gen_wxDialog))
    {
        sub_menu->AppendSeparator();
        menu_item = sub_menu->Append(MenuNEW_CHILD_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
        menu_item->SetBitmap(GetInternalImage("stddialogbuttonsizer"));
    }

    AppendSubMenu(sub_menu, "Add sizer");
}

void NavPopupMenu::MenuAddMoveCommands(Node* node)
{
    if (node->isGen(gen_Project) || node->isGen(gen_Images) || node->isGen(gen_embedded_image) || node->isGen(gen_Data) ||
        node->isGen(gen_data_string))
    {
        return;
    }

    wxMenuItem* menu_item;
    wxMenu* sub_menu;
    auto dpi_size = wxGetFrame().FromDIP(wxSize(16, 16));

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_UP, wxART_MENU));
    menu_item = sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_DOWN, wxART_MENU));

    auto gen = node->getGenerator();
    if (!m_is_parent_toolbar && gen && gen->CanChangeParent(node))
    {
        menu_item = sub_menu->Append(MenuMOVE_LEFT, "Left\tAlt+Left", "Moves selected item left");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_BACK, wxART_MENU));
        menu_item = sub_menu->Append(MenuMOVE_RIGHT, "Right\tAlt+Right", "Moves selected item right");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_FORWARD, wxART_MENU));
    }
    AppendSubMenu(sub_menu, "Move");

    if (!m_is_parent_toolbar && gen && gen->CanChangeParent(node))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_PARENT_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_PARENT_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_PARENT_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_PARENT_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

        AppendSubMenu(sub_menu, "&Move into new sizer");
    }
    else if (node->isForm())
    {
        menu_item = sub_menu->Append(MenuNEW_PARENT_FOLDER, "Move into new folder");
        menu_item->SetBitmap(GetInternalImage("folder"));
    }

    if (node->isGen(gen_wxRadioButton))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_2STATE_CHECKBOX, "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item = sub_menu->Append(MenuChangeTo_3STATE_CHECKBOX, "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxCheckBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_3STATE_CHECKBOX, "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        menu_item = sub_menu->Append(MenuChangeTo_RADIO_BUTTON, "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_Check3State))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_2STATE_CHECKBOX, "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item = sub_menu->Append(MenuChangeTo_RADIO_BUTTON, "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxChoice))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_COMBO_BOX, "wxComboBox");
        menu_item->SetBitmap(GetInternalImage("wxComboBox"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOX, "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxComboBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOX, "wxChoice");
        menu_item->SetBitmap(GetInternalImage("wxChoice"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOX, "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxListBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOX, "wxChoice");
        menu_item->SetBitmap(GetInternalImage("wxChoice"));
        menu_item = sub_menu->Append(MenuChangeTo_COMBO_BOX, "wxComboBox");
        menu_item->SetBitmap(GetInternalImage("wxComboBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxAuiNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetInternalImage("wxChoicebook"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetInternalImage("wxListbook"));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetInternalImage("wxNotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxChoicebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetInternalImage("auinotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetInternalImage("wxListbook"));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetInternalImage("wxNotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxListbook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetInternalImage("auinotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetInternalImage("wxChoicebook"));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetInternalImage("wxNotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetInternalImage("auinotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetInternalImage("wxChoicebook"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetInternalImage("wxListbook"));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    else if (node->isGen(gen_wxSimplebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetInternalImage("auinotebook"));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetInternalImage("wxChoicebook"));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetInternalImage("wxListbook"));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetInternalImage("wxNotebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
}

void NavPopupMenu::MenuAddStandardCommands(Node* node)
{
    if (node->isGen(gen_Images) || node->isGen(gen_embedded_image) || node->isGen(gen_Data) || node->isGen(gen_data_string))
    {
        auto* menu_item = Append(wxID_DELETE);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));

        m_isPasteAllowed = false;
        return;
    }

    m_isPasteAllowed = false;

    if (!node->isGen(gen_wxStatusBar))
    {
        auto clip_node = GetClipboardNode(false);
        if (!clip_node)
        {
            clip_node = wxGetFrame().getClipboardPtr();
        }

        if (node->isGen(gen_Project))
        {
            auto paste_menu_item = Append(wxID_PASTE);
            paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
            if (!clip_node || !clip_node->isForm())
            {
                paste_menu_item->Enable(false);
                m_isPasteAllowed = false;
            }

            // There are no other standard commands for a project
            return;
        }
        m_isPasteAllowed = (clip_node ? true : false);
    }

    wxMenuItem* menu_item;
    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));
    if (!node->isGen(gen_wxStatusBar))
    {
        auto paste_menu_item = Append(wxID_PASTE);
        paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
        if (!m_isPasteAllowed)
        {
            paste_menu_item->Enable(false);
        }
    }
    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));
    if (!node->isGen(gen_wxStatusBar))
    {
        Append(MenuDUPLICATE, "Duplicate");
    }
}

void NavPopupMenu::CreateSizerParent(Node* node, tt_string_view widget)
{
    auto parent = node->getParent();
    if (!parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user know why.")
        return;
    }

    auto childPos = parent->getChildPosition(node);

    if (!parent->isFormParent())
    {
        while (parent && !parent->isSizer())
        {
            parent = parent->getParent();
        }
    }

    if (!parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user know why.")
        return;
    }

    if (parent->isGen(gen_folder) || parent->isGen(gen_sub_folder))
        widget = "sub_folder";

    // Avoid the temptation to set new_parent to the raw pointer so that .get() doesn't have to be called below. Doing so
    // will result in the reference count being decremented before we are done hooking it up, and you end up crashing.

    auto new_parent = NodeCreation.createNode(widget, parent);
    if (new_parent)
    {
        wxGetFrame().Freeze();
        tt_string undo_string("Insert new ");
        if (widget == "folder" || widget == "sub_folder")
            undo_string << "folder";
        else
            undo_string << "sizer";
        wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(new_parent.get(), parent, undo_string, childPos));

        // InsertNodeAction does not fire the creation event since that's usually handled by the caller as needed. We don't
        // want to fire an event because we don't want the Mockup or Code panels to update until we have changed the parent.
        // However we *do* need to let the navigation panel know that a new node has been added.

        wxGetFrame().getNavigationPanel()->InsertNode(new_parent.get());

        wxGetFrame().PushUndoAction(std::make_shared<ChangeParentAction>(node, new_parent.get()));
        wxGetFrame().SelectNode(node, evt_flags::fire_event | evt_flags::force_selection);
        wxGetFrame().Thaw();
    }
}

void NavPopupMenu::ChangeSizer(GenEnum::GenName new_sizer_gen)
{
    wxWindowUpdateLocker freeze(wxGetFrame().getWindow());
    wxGetFrame().PushUndoAction(std::make_shared<ChangeSizerType>(m_node, new_sizer_gen));
}

void NavPopupMenu::ChangeNode(GenEnum::GenName new_node_gen)
{
    wxWindowUpdateLocker freeze(wxGetFrame().getWindow());
    wxGetFrame().PushUndoAction(std::make_shared<ChangeNodeType>(m_node, new_node_gen));
}

void NavPopupMenu::AddToolbarCommands(Node* node)
{
    auto sub_menu = new wxMenu;
    wxMenuItem* menu_item;
    AppendSubMenu(sub_menu, "Tools");

    bool is_aui_toolbar =
        (node->getGenName() == gen_wxAuiToolBar || node->getGenName() == gen_AuiToolBar ||
         node->getParent()->getGenName() == gen_wxAuiToolBar || node->getParent()->getGenName() == gen_AuiToolBar);

    menu_item = sub_menu->Append(MenuADD_TOOL, "Tool (normal, check, radio)");
    menu_item->SetBitmap(GetInternalImage("tool"));
    if (!is_aui_toolbar)
    {
        menu_item = sub_menu->Append(MenuADD_TOOL_DROPDOWN, "Dropdown");
        menu_item->SetBitmap(GetInternalImage("tool_dropdown"));
    }

    if (is_aui_toolbar)
    {
        menu_item = sub_menu->Append(MenuADD_TOOL_LABEL, "Label");
        menu_item->SetBitmap(GetInternalImage("wxStaticText"));
    }

    sub_menu->AppendSeparator();
    menu_item = sub_menu->Append(MenuADD_TOOL_COMBOBOX, "Combobox");
    menu_item->SetBitmap(GetInternalImage("wxComboBox"));
    menu_item = sub_menu->Append(MenuADD_TOOL_SLIDER, "Slider");
    menu_item->SetBitmap(GetInternalImage("slider"));
    menu_item = sub_menu->Append(MenuADD_TOOL_SPINCTRL, "Spin control");
    menu_item->SetBitmap(GetInternalImage("spin_ctrl"));
    sub_menu->AppendSeparator();

    menu_item = sub_menu->Append(MenuADD_TOOL_SEPARATOR, "Separator");
    menu_item->SetBitmap(GetInternalImage("toolseparator"));
    if (!is_aui_toolbar)
    {
        menu_item = sub_menu->Append(MenuADD_TOOL_STRETCHABLE_SPACE, "Stretchable space");
        menu_item->SetBitmap(GetInternalImage("toolStretchable"));
    }
    else
    {
        menu_item = sub_menu->Append(MenuADD_TOOL_SPACER, "Spacer");
        menu_item->SetBitmap(GetInternalImage("toolspacer"));
        menu_item = sub_menu->Append(MenuADD_TOOL_STRETCHABLE_SPACER, "Stretchable spacer");
        menu_item->SetBitmap(GetInternalImage("toolStretchable"));
    }

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().createToolNode(gen_wxComboBox);
        },
        MenuADD_TOOL_COMBOBOX);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().createToolNode(gen_wxSlider);
        },
        MenuADD_TOOL_SLIDER);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().createToolNode(gen_wxSpinCtrl);
        },
        MenuADD_TOOL_SPINCTRL);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().createToolNode(gen_toolSeparator);
        },
        MenuADD_TOOL_SEPARATOR);

    if (!is_aui_toolbar)
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_tool);
            },
            MenuADD_TOOL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_tool_dropdown);
            },
            MenuADD_TOOL_DROPDOWN);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_toolStretchable);
            },
            MenuADD_TOOL_STRETCHABLE_SPACE);
    }
    else
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_auitool);
            },
            MenuADD_TOOL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_auitool_label);
            },
            MenuADD_TOOL_LABEL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_auitool_spacer);
            },
            MenuADD_TOOL_SPACER);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().createToolNode(gen_auitool_stretchable);
            },
            MenuADD_TOOL_STRETCHABLE_SPACER);
    }
}

void NavPopupMenu::AddSeparatorIfNeeded()
{
    if (auto count = GetMenuItemCount(); count)
    {
        auto* menu_item = FindItemByPosition(count - 1);
        if (menu_item->GetKind() != wxITEM_SEPARATOR)
        {
            AppendSeparator();
        }
    }
}
