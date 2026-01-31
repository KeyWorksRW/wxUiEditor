/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>   // wxArtProvider class
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "navpopupmenu.h"  // NavPopupMenu

#include "base_generator.h"  // BaseGenerator -- Base widget generator class
#include "bitmaps.h"         // Contains various images handling functions
#include "clipboard.h"       // Handles reading and writing OS clipboard data
#include "gen_common.h"      // Common component functions
#include "gen_enums.h"
#include "mainframe.h"     // MainFrame -- Main window frame
#include "nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"         // Utility functions that work with properties

NavPopupMenu::NavPopupMenu(Node* node) : m_node(node)
{
    if (!node)
    {
        ASSERT(node);
        return;  // theoretically impossible, but don't crash if it happens
    }

    m_parent = node->get_Parent();
    if (!m_parent)  // this would mean we were created with the project selected
    {
        CreateCommonMenu();
        return;
    }

    if (m_parent->is_ToolBar())
    {
        m_is_parent_toolbar = true;
    }

    if (m_node->is_Sizer())
    {
        CreateSizerMenu();
    }
    else
    {
        CreateCommonMenu();
    }
}

void NavPopupMenu::OnMenuEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MenuNEW_ITEM:
            if (m_tool_name < gen_name_array_size)
            {
                if (m_node->is_Type(type_bookpage) || m_node->is_Type(type_wizardpagesimple))
                {
                    if (m_child && m_child->is_Sizer())
                    {
                        m_child = m_child->get_Parent();
                    }
                }

                if (m_child)
                    m_child->CreateToolNode(m_tool_name);
                else
                    wxGetFrame().CreateToolNode(m_tool_name);
            }
            break;

        case MenuNEW_COLUMN:
            if (m_tool_name == gen_wxTreeListCtrl)
            {
                if (m_child)
                    m_child->CreateToolNode(gen_TreeListCtrlColumn);
                else
                    wxGetFrame().CreateToolNode(gen_TreeListCtrlColumn);
            }
            break;

        case MenuNEW_SIBLING_BOX_SIZER:
            m_parent->CreateToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_SIBLING_STATIC_SIZER:
            m_parent->CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_SIBLING_WRAP_SIZER:
            m_parent->CreateToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_SIBLING_GRID_SIZER:
            m_parent->CreateToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_SIBLING_FLEX_GRID_SIZER:
            m_parent->CreateToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_SIBLING_GRIDBAG_SIZER:
            m_parent->CreateToolNode(gen_wxGridBagSizer);
            break;

        case MenuNEW_SIBLING_STD_DIALG_BTNS:
            m_parent->CreateToolNode(gen_wxStdDialogButtonSizer);
            break;

        case MenuNEW_SIBLING_SPACER:
            m_parent->CreateToolNode(gen_spacer);
            break;

        case MenuNEW_CHILD_BOX_SIZER:
            m_sizer_node->CreateToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_CHILD_STATIC_SIZER:
            m_sizer_node->CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_CHILD_WRAP_SIZER:
            m_sizer_node->CreateToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_CHILD_GRID_SIZER:
            m_sizer_node->CreateToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_CHILD_FLEX_GRID_SIZER:
            m_sizer_node->CreateToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_CHILD_GRIDBAG_SIZER:
            m_sizer_node->CreateToolNode(gen_wxGridBagSizer);
            break;

        case MenuNEW_CHILD_STD_DIALG_BTNS:
            wxGetFrame().CreateToolNode(gen_wxStdDialogButtonSizer);
            break;

        case MenuNEW_CHILD_SPACER:
            wxGetFrame().CreateToolNode(gen_spacer);
            break;

        case MenuNEW_TOOLBAR:
            wxGetFrame().CreateToolNode(gen_wxToolBar);
            break;

        case MenuNEW_INFOBAR:
            wxGetFrame().CreateToolNode(gen_wxInfoBar);
            break;

        case MenuADD_MENU:
            wxGetFrame().CreateToolNode(gen_wxMenu);
            break;

        case MenuADD_TOOL_SEPARATOR:
            if (m_child)
                m_child->CreateToolNode(gen_toolSeparator);
            else
                wxGetFrame().CreateToolNode(gen_toolSeparator);
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
            CreateSizerParent("wxBoxSizer");
            break;

        case MenuNEW_PARENT_STATIC_SIZER:
            CreateSizerParent("wxStaticBoxSizer");
            break;

        case MenuNEW_PARENT_WRAP_SIZER:
            CreateSizerParent("wxWrapSizer");
            break;

        case MenuNEW_PARENT_GRID_SIZER:
            CreateSizerParent("wxGridSizer");
            break;

        case MenuNEW_PARENT_FLEX_GRID_SIZER:
            CreateSizerParent("wxFlexGridSizer");
            break;

        case MenuNEW_PARENT_GRIDBAG_SIZER:
            CreateSizerParent("wxGridBagSizer");
            break;

        case MenuNEW_PARENT_FOLDER:
            CreateSizerParent("folder");
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

        case MenuCompareCode:
            {
                // Because this immediately creates a dialog, we need to be sure that MainFrame
                // has the correct node selected first before the dialog queries it.
                wxGetFrame().SelectNode(m_node);
                wxCommandEvent dummy;
                wxGetMainFrame()->OnCodeCompare(dummy);
            }
            break;

        case MenuSingleGenCpp:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleCpp(dummy);
            }
            break;

        case MenuSingleGenPerl:
            {
                OnGenerateSingleLanguage(GEN_LANG_PERL);
            }
            break;

        case MenuSingleGenPython:
            {
                OnGenerateSingleLanguage(GEN_LANG_PYTHON);
            }
            break;

        case MenuSingleGenRuby:
            {
                OnGenerateSingleLanguage(GEN_LANG_RUBY);
            }
            break;

        case MenuSingleGenXRC:
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleXRC(dummy);
            }
            break;

        case MenuADD_PAGE:
            if (m_node->is_Gen(gen_BookPage))
            {
                m_node->get_Parent()->CreateToolNode(gen_BookPage);
            }
            if (m_node->is_Gen(gen_wxWizardPageSimple))
            {
                m_node->get_Parent()->CreateToolNode(gen_wxWizardPageSimple);
            }
            if (!m_node->is_Gen(gen_BookPage) && !m_node->is_Gen(gen_wxWizardPageSimple))
            {
                wxGetFrame().CreateToolNode(gen_wxPanel);
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
            if (m_node && !m_node->is_Gen(gen_Project))
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

void NavPopupMenu::CreateSizerMenu()
{
    // This needs to be added first to cover all menu ids that aren't specially bound to an id.
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    m_sizer_node = m_node;
    wxMenuItem* menu_item;
    wxMenu* sub_menu;

    auto& dpi_size = wxGetFrame().GetMenuDpiSize();
    bool isTopSizer = (m_parent->is_Form() || m_parent->is_Container());

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
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
        menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));
        AppendSubMenu(sub_menu, "Add sibling sizer");

        AppendSeparator();
        MenuAddMoveCommands();
    }

    if (m_node->is_Gen(gen_wxBoxSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxFlexGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxWrapSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }

    AddSeparatorIfNeeded();
    MenuAddStandardCommands();
}

void NavPopupMenu::CreateCommonMenu()
{
    // This needs to be added first to cover all menu ids that aren't specially bound to an id.
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    MenuAddCommands();
    MenuAddMoveCommands();
    MenuAddStandardCommands();
}

void NavPopupMenu::MenuAddCommands()
{
    wxMenuItem* menu_item = nullptr;
    const auto& dpi_size = wxGetFrame().GetMenuDpiSize();

    if (wxGetApp().isTestingMenuEnabled())
    {
        if (m_node->is_Form())
        {
            int count = 0;
            if (m_node->HasValue(prop_base_file))
            {
                Append(MenuSingleGenCpp, "Generate C++ for this form");
                ++count;
            }
            if (m_node->HasValue(prop_python_file))
            {
                Append(MenuSingleGenPython, "Generate Python for this form");
                ++count;
            }
            if (m_node->HasValue(prop_ruby_file))
            {
                Append(MenuSingleGenRuby, "Generate Ruby for this form");
                ++count;
            }
            if (m_node->HasValue(prop_xrc_file))
            {
                Append(MenuSingleGenXRC, "Generate XRC for this form");
                ++count;
            }
            if (m_node->HasValue(prop_perl_file))
            {
                Append(MenuSingleGenPerl, "Generate Perl for this form");
                ++count;
            }

            if (count)
            {
                Append(MenuCompareCode, "Compare Code for this form");
                AppendSeparator();
            }
        }
    }

    if (m_node->is_Form() || m_node->is_Gen(gen_Images) || m_node->is_Gen(gen_embedded_image))
    {
        if (m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            menu_item = Append(MenuADD_PAGE, "Add Page");
            menu_item->SetBitmap(GetSvgImage("book_page", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_BookPage);
                },
                MenuADD_PAGE);
            return;
        }
        if (m_node->is_Gen(gen_Data))
        {
            menu_item = Append(MenuADD_DATA_STRING, "Add File");
            menu_item->SetBitmap(GetSvgImage("text_file", dpi_size));
            menu_item = Append(MenuADD_DATA_XML, "Add XML File");
            menu_item->SetBitmap(GetSvgImage("xml_file", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_string);
                },
                MenuADD_DATA_STRING);
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                MenuADD_DATA_XML);
            return;
        }
        if (m_node->is_Gen(gen_Images))
        {
            menu_item = Append(MenuADD_IMAGE, "Add Image");
            menu_item->SetBitmap(GetSvgImage("bitmap", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_embedded_image);
                },
                MenuADD_IMAGE);
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                MenuADD_DATA_XML);
            return;
        }
        if (!m_node->is_Gen(gen_wxWizard) && !m_node->is_ToolBar())
        {
            return;
        }
    }

    if (m_node->is_Gen(gen_wxStatusBar) || m_node->is_Gen(gen_embedded_image))
    {
        return;
    }

    if (auto gen = m_node->get_Generator(); gen)
    {
        if (!gen->PopupMenuAddCommands(this, m_node))
        {
            return;
        }
        AddSeparatorIfNeeded();
    }

    bool add_sizer = true;

    switch (m_node->get_GenName())
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
                    wxGetFrame().CreateToolNode(gen_BookPage);
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
                    wxGetFrame().CreateToolNode(gen_wxRibbonPage);
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
                    wxGetFrame().CreateToolNode(gen_wxRibbonPanel);
                },
                MenuADD_RIBBON_PANEL);
            break;

        case gen_wxRibbonPanel:
            add_sizer = false;
            if (m_node->get_ChildCount())
            {
                Append(MenuADD_RIBBON_PANEL, "Add Panel");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_wxRibbonPanel);
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
                        wxGetFrame().CreateToolNode(gen_wxRibbonButtonBar);
                    },
                    MenuADD_RIBBON_BUTTONBAR);

                Append(MenuADD_RIBBON_TOOLBAR, "Add Tool Bar");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_wxRibbonToolBar);
                    },
                    MenuADD_RIBBON_TOOLBAR);

                Append(MenuADD_RIBBON_GALLERY, "Add Gallery");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_wxRibbonGallery);
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
                    wxGetFrame().CreateToolNode(gen_ribbonButton);
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
                    wxGetFrame().CreateToolNode(gen_ribbonGalleryItem);
                },
                MenuADD_RIBBON_GALLERY_ITEM);
            break;

        case gen_AuiToolBar:
        case gen_wxAuiToolBar:
        case gen_auitool:
            add_sizer = false;
            AddToolbarCommands(m_node);
            break;

        case gen_wxToolBar:
        case gen_ToolBar:
        case gen_tool:
        case gen_toolSeparator:
        case gen_toolStretchable:
            add_sizer = false;
            AddToolbarCommands(m_node);
            break;

        case gen_wxMenuBar:
        case gen_MenuBar:
            add_sizer = false;
            Append(MenuADD_MENU, "Add Menu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxMenu);
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
                    wxGetFrame().CreateToolNode(gen_wxMenuItem);
                },
                MenuADD_MENUITEM);

            Append(MenuADD_SUBMENU, "Add Submenu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_submenu);
                },
                MenuADD_SUBMENU);

            Append(MenuADD_MENU_SEPARATOR, "Add Separator");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_separator);
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
                    wxGetFrame().CreateToolNode(gen_propGridPage);
                },
                MenuADD_PROPGRID_PAGE);
            break;

        case gen_wxPropertyGrid:
        case gen_propGridPage:
        case gen_propGridCategory:
        case gen_propGridItem:
            add_sizer = false;
            if (!m_node->is_Gen(gen_propGridCategory) && !m_node->is_Gen(gen_propGridItem))
            {
                Append(MenuADD_PROPGRID_CATEGORY, "Add Category");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_propGridCategory);
                    },
                    MenuADD_PROPGRID_CATEGORY);
            }
            Append(MenuADD_PROPGRID_ITEM, "Add Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_propGridItem);
                },
                MenuADD_PROPGRID_ITEM);
            break;

        case gen_data_string:
        case gen_data_xml:
        case gen_data_folder:
            add_sizer = false;
            Append(MenuADD_DATA_STRING, "Add File");
            Append(MenuADD_DATA_XML, "Add XML File");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_string);
                },
                MenuADD_DATA_STRING);
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                MenuADD_DATA_XML);
            break;

        default:
            if (m_is_parent_toolbar)
            {
                add_sizer = false;
                AddToolbarCommands(m_node);
            }
            else
            {
                Append(MenuNEW_CHILD_SPACER, "Add spacer");
            }
            break;
    }

    if (add_sizer)
    {
        MenuAddChildSizerCommands(m_node);
    }

    if (!m_node->is_Gen(gen_Project))
    {
        AddSeparatorIfNeeded();
    }
}

void NavPopupMenu::MenuAddChildSizerCommands(Node* child)
{
    m_sizer_node = child;
    auto& dpi_size = wxGetFrame().GetMenuDpiSize();

    auto sub_menu = new wxMenu;
    auto menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

    if (child->is_Parent(gen_wxDialog))
    {
        sub_menu->AppendSeparator();
        menu_item = sub_menu->Append(MenuNEW_CHILD_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
        menu_item->SetBitmap(GetInternalImage("stddialogbuttonsizer"));
    }

    AppendSubMenu(sub_menu, "Add sizer");
}

void NavPopupMenu::MenuAddMoveCommands()
{
    if (m_node->is_Gen(gen_Project) || m_node->is_Gen(gen_Images) || m_node->is_Gen(gen_Data))
    {
        return;
    }

    wxMenuItem* menu_item;
    wxMenu* sub_menu;
    auto& dpi_size = wxGetFrame().GetMenuDpiSize();
    AddSeparatorIfNeeded();

    auto gen = m_node->get_Generator();

    sub_menu = new wxMenu;
    if (m_parent && m_parent->is_Gen(GenEnum::gen_wxGridBagSizer))
    {
        menu_item = sub_menu->Append(MenuMOVE_UP, "Decrease Row\tAlt+Up",
                                     "Decreases row containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_UP, wxART_MENU));
        menu_item = sub_menu->Append(MenuMOVE_DOWN, "Increase Row\tAlt+Down",
                                     "Increases row containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_DOWN, wxART_MENU));

        menu_item = sub_menu->Append(MenuMOVE_LEFT, "Decrease Column\tAlt+Left",
                                     "Decreases column containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_BACK, wxART_MENU));
        menu_item = sub_menu->Append(MenuMOVE_RIGHT, "Increase Column\tAlt+Right",
                                     "Increases column containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_FORWARD, wxART_MENU));
    }
    else
    {
        menu_item = sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_UP, wxART_MENU));
        menu_item = sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_DOWN, wxART_MENU));

        if (!m_is_parent_toolbar && gen && gen->CanChangeParent(m_node))
        {
            menu_item =
                sub_menu->Append(MenuMOVE_LEFT, "Left\tAlt+Left", "Moves selected item left");
            menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_BACK, wxART_MENU));
            menu_item =
                sub_menu->Append(MenuMOVE_RIGHT, "Right\tAlt+Right", "Moves selected item right");
            menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_FORWARD, wxART_MENU));
        }
    }
    AppendSubMenu(sub_menu, "Move");

    if (m_node->is_Gen(gen_folder) || m_node->is_Gen(gen_sub_folder) ||
        m_node->is_Gen(gen_data_folder))
    {
        // Folders can only move up, down, left or right. No other move operations can be done on a
        // folder.
        return;
    }

    if (!m_is_parent_toolbar && gen && gen->CanChangeParent(m_node))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_PARENT_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(MenuNEW_PARENT_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

        AppendSubMenu(sub_menu, "&Move into new sizer");
    }
    if (m_node->is_Form())
    {
        menu_item = sub_menu->Append(MenuNEW_PARENT_FOLDER, "Move into new folder");
        menu_item->SetBitmap(GetInternalImage("folder"));
    }

    if (m_node->is_Gen(gen_wxRadioButton))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_2STATE_CHECKBOX, "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item = sub_menu->Append(MenuChangeTo_3STATE_CHECKBOX, "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxCheckBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_3STATE_CHECKBOX, "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        menu_item = sub_menu->Append(MenuChangeTo_RADIO_BUTTON, "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_Check3State))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_2STATE_CHECKBOX, "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item = sub_menu->Append(MenuChangeTo_RADIO_BUTTON, "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxChoice))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_COMBO_BOX, "wxComboBox");
        menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOX, "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxComboBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOX, "wxChoice");
        menu_item->SetBitmap(GetSvgImage("wxChoice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOX, "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxListBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOX, "wxChoice");
        menu_item->SetBitmap(GetSvgImage("wxChoice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_COMBO_BOX, "wxComboBox");
        menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxAuiNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxChoicebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxListbook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_SIMPLE_BOOK, "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxSimplebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_AUI_BOOK, "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_CHOICE_BOOK, "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_LIST_BOOK, "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(MenuChangeTo_NOTE_BOOK, "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    return;
}

void NavPopupMenu::MenuAddStandardCommands()
{
    auto& dpi_size = wxGetFrame().GetMenuDpiSize();

    AddSeparatorIfNeeded();
    m_isPasteAllowed = false;
    if (m_node->is_Gen(gen_embedded_image))
    {
        auto* menu_item = Append(wxID_DELETE);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));
        return;
    }
    if (m_node->is_Gen(gen_wxStatusBar))
    {
        auto* menu_item = Append(wxID_CUT);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_CUT, wxART_MENU));

        menu_item = Append(wxID_COPY);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));

        menu_item = Append(wxID_DELETE);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));
        return;
    }

    auto clip_node = GetClipboardNode(false);
    if (!clip_node)
    {
        clip_node = wxGetFrame().getClipboardPtr();
    }
    m_isPasteAllowed = (clip_node ? true : false);

    if (m_node->is_Gen(gen_Project))
    {
        auto paste_menu_item = Append(wxID_PASTE);
        paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
        if (!clip_node || !clip_node->is_Form())
        {
            paste_menu_item->Enable(false);
            m_isPasteAllowed = false;
        }

        // There are no other standard commands for a project
        return;
    }

    wxMenuItem* menu_item;
    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_CUT, wxART_MENU));

    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));

    auto paste_menu_item = Append(wxID_PASTE);
    paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
    paste_menu_item->Enable((clip_node ? true : false));

    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));

    AddSeparatorIfNeeded();
    if (!m_node->is_Gen(gen_Images) && !m_node->is_Gen(gen_Data))
    {
        menu_item = Append(MenuDUPLICATE, "Duplicate");
        menu_item->SetBitmap(GetSvgImage("duplicate", dpi_size));
    }

    menu_item = Append(MenuInsertWidget, "Insert Widget...");
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_EDIT, wxART_MENU));
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent& event)
        {
            wxGetFrame().OnInsertWidget(event);
        },
        MenuInsertWidget);
}

void NavPopupMenu::CreateSizerParent(std::string_view widget)
{
    if (!m_parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it
        // didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user "
                 "know why.")
        return;
    }

    auto childPos = m_parent->get_ChildPosition(m_node);

    if (!m_parent->is_FormParent())
    {
        while (m_parent && !m_parent->is_Sizer())
        {
            m_parent = m_parent->get_Parent();
        }
    }

    if (!m_parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it
        // didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user "
                 "know why.")
        return;
    }

    if (m_parent->is_Gen(gen_folder) || m_parent->is_Gen(gen_sub_folder))
        widget = "sub_folder";

    // Avoid the temptation to set new_parent to the raw pointer so that .get() doesn't have to be
    // called below. Doing so will result in the reference count being decremented before we are
    // done hooking it up, and you end up crashing.

    auto new_parent = NodeCreation.CreateNode(widget, m_parent).first;
    if (new_parent)
    {
        wxGetFrame().Freeze();
        wxue::string undo_string("Insert new ");
        if (widget == "folder" || widget == "sub_folder")
            undo_string << "folder";
        else
            undo_string << "sizer";
        if (!m_parent->is_Gen(gen_wxGridBagSizer))
        {
            wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(
                new_parent.get(), m_parent, undo_string, childPos));
        }
        else
        {
            auto new_child = NodeCreation.MakeCopy(m_node);
            undo_string = "Remove widget";
            wxGetFrame().PushUndoAction(std::make_shared<RemoveNodeAction>(m_node, undo_string));
            new_parent->AdoptChild(new_child->get_SharedPtr());
            new_parent->set_value(prop_column, new_child->as_string(prop_column));
            new_parent->set_value(prop_row, new_child->as_string(prop_row));
            new_parent->set_value(prop_colspan, new_child->as_string(prop_colspan));
            new_parent->set_value(prop_rowspan, new_child->as_string(prop_rowspan));
            // wxGetFrame().FireDeletedEvent(new_child);
            wxGetFrame().PushUndoAction(std::make_shared<AppendGridBagAction>(
                new_parent.get(), m_parent, (to_int) childPos));
            wxGetFrame().SelectNode(new_child, evt_flags::fire_event | evt_flags::force_selection);
            wxGetFrame().Thaw();
            return;
        }

        // InsertNodeAction does not fire the creation event since that's usually handled by the
        // caller as needed. We don't want to fire an event because we don't want the Mockup or Code
        // panels to update until we have changed the parent. However we *do* need to let the
        // navigation panel know that a new node has been added.

        wxGetFrame().getNavigationPanel()->InsertNode(new_parent.get());

        wxGetFrame().PushUndoAction(std::make_shared<ChangeParentAction>(m_node, new_parent.get()));
        wxGetFrame().SelectNode(m_node, evt_flags::fire_event | evt_flags::force_selection);
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
    auto& dpi_size = wxGetFrame().GetMenuDpiSize();

    AppendSubMenu(sub_menu, "Tools");

    bool is_aui_toolbar =
        (node->get_GenName() == gen_wxAuiToolBar || node->get_GenName() == gen_AuiToolBar ||
         node->get_Parent()->get_GenName() == gen_wxAuiToolBar ||
         node->get_Parent()->get_GenName() == gen_AuiToolBar);

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
        menu_item->SetBitmap(GetSvgImage("wxStaticText", dpi_size));
    }

    sub_menu->AppendSeparator();
    menu_item = sub_menu->Append(MenuADD_TOOL_COMBOBOX, "Combobox");
    menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
    menu_item = sub_menu->Append(MenuADD_TOOL_SLIDER, "Slider");
    menu_item->SetBitmap(GetSvgImage("slider", dpi_size));
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
            wxGetFrame().CreateToolNode(gen_wxComboBox);
        },
        MenuADD_TOOL_COMBOBOX);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxSlider);
        },
        MenuADD_TOOL_SLIDER);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxSpinCtrl);
        },
        MenuADD_TOOL_SPINCTRL);
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_toolSeparator);
        },
        MenuADD_TOOL_SEPARATOR);

    if (!is_aui_toolbar)
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_tool);
            },
            MenuADD_TOOL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_tool_dropdown);
            },
            MenuADD_TOOL_DROPDOWN);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_toolStretchable);
            },
            MenuADD_TOOL_STRETCHABLE_SPACE);
    }
    else
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool);
            },
            MenuADD_TOOL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_label);
            },
            MenuADD_TOOL_LABEL);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_spacer);
            },
            MenuADD_TOOL_SPACER);
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_stretchable);
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
