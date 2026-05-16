/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <utility>  // std::ignore. std::tp_underlying

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
        case std::to_underlying(Menu::NewItem):
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
                {
                    std::ignore = m_child->CreateToolNode(m_tool_name);
                }
                else
                {
                    wxGetFrame().CreateToolNode(m_tool_name);
                }
            }
            break;

        case std::to_underlying(Menu::NewColumn):
            if (m_tool_name == gen_wxTreeListCtrl)
            {
                if (m_child)
                {
                    std::ignore = m_child->CreateToolNode(gen_TreeListCtrlColumn);
                }
                else
                {
                    wxGetFrame().CreateToolNode(gen_TreeListCtrlColumn);
                }
            }
            break;

        case std::to_underlying(Menu::NewSiblingBoxSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxBoxSizer);
            break;

        case std::to_underlying(Menu::NewSiblingStaticSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case std::to_underlying(Menu::NewSiblingWrapSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxWrapSizer);
            break;

        case std::to_underlying(Menu::NewSiblingGridSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxGridSizer);
            break;

        case std::to_underlying(Menu::NewSiblingFlexGridSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxFlexGridSizer);
            break;

        case std::to_underlying(Menu::NewSiblingGridbagSizer):
            std::ignore = m_parent->CreateToolNode(gen_wxGridBagSizer);
            break;

        case std::to_underlying(Menu::NewSiblingStdDialgBtns):
            std::ignore = m_parent->CreateToolNode(gen_wxStdDialogButtonSizer);
            break;

        case std::to_underlying(Menu::NewSiblingSpacer):
            std::ignore = m_parent->CreateToolNode(gen_spacer);
            break;

        case std::to_underlying(Menu::NewChildBoxSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxBoxSizer);
            break;

        case std::to_underlying(Menu::NewChildStaticSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case std::to_underlying(Menu::NewChildWrapSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxWrapSizer);
            break;

        case std::to_underlying(Menu::NewChildGridSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxGridSizer);
            break;

        case std::to_underlying(Menu::NewChildFlexGridSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxFlexGridSizer);
            break;

        case std::to_underlying(Menu::NewChildGridbagSizer):
            std::ignore = m_sizer_node->CreateToolNode(gen_wxGridBagSizer);
            break;

        case std::to_underlying(Menu::NewChildStdDialgBtns):
            wxGetFrame().CreateToolNode(gen_wxStdDialogButtonSizer);
            break;

        case std::to_underlying(Menu::NewChildSpacer):
            wxGetFrame().CreateToolNode(gen_spacer);
            break;

        case std::to_underlying(Menu::NewToolbar):
            wxGetFrame().CreateToolNode(gen_wxToolBar);
            break;

        case std::to_underlying(Menu::NewInfobar):
            wxGetFrame().CreateToolNode(gen_wxInfoBar);
            break;

        case std::to_underlying(Menu::AddMenu):
            wxGetFrame().CreateToolNode(gen_wxMenu);
            break;

        case std::to_underlying(Menu::AddToolSeparator):
            if (m_child)
            {
                std::ignore = m_child->CreateToolNode(gen_toolSeparator);
            }
            else
            {
                wxGetFrame().CreateToolNode(gen_toolSeparator);
            }
            break;

        case std::to_underlying(Menu::ExpandAll):
            wxGetFrame().getNavigationPanel()->ExpandCollapse(m_node);
            wxGetFrame().SelectNode(m_node);
            break;

        case wxID_CUT:
            wxGetFrame().CutNode(m_node);
            break;

        case wxID_COPY:
            m_node->CopyNode();
            break;

        case wxID_PASTE:
            wxGetFrame().PasteNode(m_node);
            break;

        case std::to_underlying(Menu::Duplicate):
            m_node->DuplicateNode();
            break;

        case wxID_DELETE:
            wxGetFrame().DeleteNode(m_node);
            break;

        case std::to_underlying(Menu::MoveUp):
            m_node->MoveNode(MoveDirection::Up);
            break;

        case std::to_underlying(Menu::MoveDown):
            m_node->MoveNode(MoveDirection::Down);
            break;

        case std::to_underlying(Menu::MoveRight):
            m_node->MoveNode(MoveDirection::Right);
            break;

        case std::to_underlying(Menu::MoveLeft):
            m_node->MoveNode(MoveDirection::Left);
            break;

        case std::to_underlying(Menu::MergeSizerChildren):
            m_node->MoveSizerChildrenToParent();
            break;

        case std::to_underlying(Menu::NewParentBoxSizer):
            CreateSizerParent("wxBoxSizer");
            break;

        case std::to_underlying(Menu::NewParentStaticSizer):
            CreateSizerParent("wxStaticBoxSizer");
            break;

        case std::to_underlying(Menu::NewParentWrapSizer):
            CreateSizerParent("wxWrapSizer");
            break;

        case std::to_underlying(Menu::NewParentGridSizer):
            CreateSizerParent("wxGridSizer");
            break;

        case std::to_underlying(Menu::NewParentFlexGridSizer):
            CreateSizerParent("wxFlexGridSizer");
            break;

        case std::to_underlying(Menu::NewParentGridbagSizer):
            CreateSizerParent("wxGridBagSizer");
            break;

        case std::to_underlying(Menu::NewParentFolder):
            CreateSizerParent("folder");
            break;

        case std::to_underlying(Menu::ChangeToFlexGridSizer):
            ChangeSizer(gen_wxFlexGridSizer);
            break;

        case std::to_underlying(Menu::ChangeToAuiBook):
            ChangeNode(gen_wxAuiNotebook);
            break;

        case std::to_underlying(Menu::ChangeToChoiceBook):
            ChangeNode(gen_wxChoicebook);
            break;

        case std::to_underlying(Menu::ChangeToListBook):
            ChangeNode(gen_wxListbook);
            break;

        case std::to_underlying(Menu::ChangeToNoteBook):
            ChangeNode(gen_wxNotebook);
            break;

        case std::to_underlying(Menu::ChangeToSimpleBook):
            ChangeNode(gen_wxSimplebook);
            break;

        case std::to_underlying(Menu::ChangeTo2StateCheckbox):
            ChangeNode(gen_wxCheckBox);
            break;

        case std::to_underlying(Menu::ChangeTo3StateCheckbox):
            ChangeNode(gen_Check3State);
            break;

        case std::to_underlying(Menu::ChangeToRadioButton):
            ChangeNode(gen_wxRadioButton);
            break;

        case std::to_underlying(Menu::ChangeToChoiceBox):
            ChangeNode(gen_wxChoice);
            break;

        case std::to_underlying(Menu::ChangeToComboBox):
            ChangeNode(gen_wxComboBox);
            break;

        case std::to_underlying(Menu::ChangeToListBox):
            ChangeNode(gen_wxListBox);
            break;

        case std::to_underlying(Menu::ChangeToGridSizer):
            ChangeSizer(gen_wxGridSizer);
            break;

        case std::to_underlying(Menu::ChangeToStaticSizer):
            ChangeSizer(gen_wxStaticBoxSizer);
            break;

        case std::to_underlying(Menu::ChangeToWrapSizer):
            ChangeSizer(gen_wxWrapSizer);
            break;

        case std::to_underlying(Menu::CompareCode):
            {
                // Because this immediately creates a dialog, we need to be sure that MainFrame
                // has the correct node selected first before the dialog queries it.
                wxGetFrame().SelectNode(m_node);
                wxCommandEvent dummy;
                wxGetMainFrame()->OnCodeCompare(dummy);
            }
            break;

        case std::to_underlying(Menu::SingleGenCpp):
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleCpp(dummy);
            }
            break;

        case std::to_underlying(Menu::SingleGenPython):
            {
                OnGenerateSingleLanguage(GEN_LANG_PYTHON);
            }
            break;

        case std::to_underlying(Menu::SingleGenRuby):
            {
                OnGenerateSingleLanguage(GEN_LANG_RUBY);
            }
            break;

        case std::to_underlying(Menu::SingleGenXRC):
            {
                wxCommandEvent dummy;
                wxGetMainFrame()->OnGenSingleXRC(dummy);
            }
            break;

        case std::to_underlying(Menu::AddPage):
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
            {
                event.Enable(true);
            }
            else
            {
                event.Enable(wxGetFrame().CanCopyNode());
            }
            break;

        case std::to_underlying(Menu::MoveUp):
            Enable(std::to_underlying(Menu::MoveUp), m_node->MoveNode(MoveDirection::Up, true));
            break;

        case std::to_underlying(Menu::MoveDown):
            Enable(std::to_underlying(Menu::MoveDown), m_node->MoveNode(MoveDirection::Down, true));
            break;

        case std::to_underlying(Menu::MoveLeft):
            Enable(std::to_underlying(Menu::MoveLeft), m_node->MoveNode(MoveDirection::Left, true));
            break;

        case std::to_underlying(Menu::MoveRight):
            Enable(std::to_underlying(Menu::MoveRight),
                   m_node->MoveNode(MoveDirection::Right, true));
            break;
    }
}

void NavPopupMenu::CreateSizerMenu()
{
    // This needs to be added first to cover all menu ids that aren't specially bound to an id.
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    m_sizer_node = m_node;
    wxMenuItem* menu_item { nullptr };
    wxMenu* sub_menu { nullptr };

    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();
    const bool isTopSizer = (m_parent->is_Form() || m_parent->is_Container());

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildBoxSizer), "wxBoxSizer");
    menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildStaticSizer), "wxStaticBoxSizer");
    menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildWrapSizer), "wxWrapSizer");
    menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildGridSizer), "wxGridSizer");
    menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
    menu_item =
        sub_menu->Append(std::to_underlying(Menu::NewChildFlexGridSizer), "wxFlexGridSizer");
    menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildGridbagSizer), "wxGridBagSizer");
    menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

    AppendSubMenu(sub_menu, "Add child sizer");

    if (!isTopSizer)
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewSiblingBoxSizer), "wxBoxSizer");
        menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewSiblingStaticSizer), "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewSiblingWrapSizer), "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewSiblingGridSizer), "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewSiblingFlexGridSizer), "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewSiblingGridbagSizer), "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));
        AppendSubMenu(sub_menu, "Add sibling sizer");

        AppendSeparator();
        MenuAddMoveCommands();
    }

    if (m_node->is_Gen(gen_wxBoxSizer))
    {
        sub_menu = new wxMenu;
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToFlexGridSizer), "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToGridSizer), "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToStaticSizer), "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToWrapSizer), "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToFlexGridSizer), "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToWrapSizer), "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxFlexGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToGridSizer), "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToWrapSizer), "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    if (m_node->is_Gen(gen_wxWrapSizer))
    {
        sub_menu = new wxMenu;
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToFlexGridSizer), "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToGridSizer), "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }

    if (m_parent && m_parent->is_Sizer())
    {
        Append(std::to_underlying(Menu::MergeSizerChildren), "Merge sizer children into parent");
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
    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();

    if (wxGetApp().isTestingMenuEnabled())
    {
        if (m_node->is_Form())
        {
            int count = 0;
            if (m_node->HasValue(prop_base_file))
            {
                Append(std::to_underlying(Menu::SingleGenCpp), "Generate C++ for this form");
                ++count;
            }
            if (m_node->HasValue(prop_python_file))
            {
                Append(std::to_underlying(Menu::SingleGenPython), "Generate Python for this form");
                ++count;
            }
            if (m_node->HasValue(prop_ruby_file))
            {
                Append(std::to_underlying(Menu::SingleGenRuby), "Generate Ruby for this form");
                ++count;
            }
            if (m_node->HasValue(prop_xrc_file))
            {
                Append(std::to_underlying(Menu::SingleGenXRC), "Generate XRC for this form");
                ++count;
            }
            if (m_node->HasValue(prop_perl_file))
            {
                Append(std::to_underlying(Menu::SingleGenPerl), "Generate Perl for this form");
                ++count;
            }

            if (count)
            {
                Append(std::to_underlying(Menu::CompareCode), "Compare Code for this form");
                AppendSeparator();
            }
        }
    }

    if (m_node->is_Form() || m_node->is_Gen(gen_Images) || m_node->is_Gen(gen_embedded_image))
    {
        if (m_node->is_Gen(gen_wxPropertySheetDialog))
        {
            menu_item = Append(std::to_underlying(Menu::AddPage), "Add Page");
            menu_item->SetBitmap(GetSvgImage("book_page", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_BookPage);
                },
                std::to_underlying(Menu::AddPage));
            return;
        }
        if (m_node->is_Gen(gen_Data))
        {
            menu_item = Append(std::to_underlying(Menu::AddDataString), "Add File");
            menu_item->SetBitmap(GetSvgImage("text_file", dpi_size));
            menu_item = Append(std::to_underlying(Menu::AddDataXml), "Add XML File");
            menu_item->SetBitmap(GetSvgImage("xml_file", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_string);
                },
                std::to_underlying(Menu::AddDataString));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                std::to_underlying(Menu::AddDataXml));
            return;
        }
        if (m_node->is_Gen(gen_Images))
        {
            menu_item = Append(std::to_underlying(Menu::AddImage), "Add Image");
            menu_item->SetBitmap(GetSvgImage("bitmap", dpi_size));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_embedded_image);
                },
                std::to_underlying(Menu::AddImage));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                std::to_underlying(Menu::AddDataXml));
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

    if (BaseGenerator* generator = m_node->get_Generator(); generator)
    {
        if (!generator->PopupMenuAddCommands(this, m_node))
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
            Append(std::to_underlying(Menu::AddPage), "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_BookPage);
                },
                std::to_underlying(Menu::AddPage));
            break;

        case gen_BookPage:
            add_sizer = true;
            Append(std::to_underlying(Menu::AddPage), "Add Page");
            break;

        case gen_wxRibbonBar:
        case gen_RibbonBar:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddRibbonPage), "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxRibbonPage);
                },
                std::to_underlying(Menu::AddRibbonPage));
            break;

        case gen_wxRibbonPage:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddRibbonPanel), "Add Panel");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxRibbonPanel);
                },
                std::to_underlying(Menu::AddRibbonPanel));
            break;

        case gen_wxRibbonPanel:
            add_sizer = false;
            if (m_node->get_ChildCount())
            {
                Append(std::to_underlying(Menu::AddRibbonPanel), "Add Panel");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_wxRibbonPanel);
                    },
                    std::to_underlying(Menu::AddRibbonPanel));
                break;
            }

            Append(std::to_underlying(Menu::AddRibbonButtonbar), "Add Button Bar");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxRibbonButtonBar);
                },
                std::to_underlying(Menu::AddRibbonButtonbar));

            Append(std::to_underlying(Menu::AddRibbonToolbar), "Add Tool Bar");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxRibbonToolBar);
                },
                std::to_underlying(Menu::AddRibbonToolbar));

            Append(std::to_underlying(Menu::AddRibbonGallery), "Add Gallery");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxRibbonGallery);
                },
                std::to_underlying(Menu::AddRibbonGallery));
            break;

        case gen_wxRibbonButtonBar:
        case gen_wxRibbonToolBar:
        case gen_ribbonButton:
        case gen_ribbonTool:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddRibbonButton), "Add Button");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_ribbonButton);
                },
                std::to_underlying(Menu::AddRibbonButton));
            break;

        case gen_wxRibbonGallery:
        case gen_ribbonGalleryItem:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddRibbonGalleryItem), "Add Gallery Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_ribbonGalleryItem);
                },
                std::to_underlying(Menu::AddRibbonGalleryItem));
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
            Append(std::to_underlying(Menu::AddMenu), "Add Menu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxMenu);
                },
                std::to_underlying(Menu::AddMenu));
            break;

        case gen_wxMenu:
        case gen_wxMenuItem:
        case gen_submenu:
        case gen_separator:
        case gen_tool_dropdown:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddMenuitem), "Add Menu Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_wxMenuItem);
                },
                std::to_underlying(Menu::AddMenuitem));

            Append(std::to_underlying(Menu::AddSubmenu), "Add Submenu");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_submenu);
                },
                std::to_underlying(Menu::AddSubmenu));

            Append(std::to_underlying(Menu::AddMenuSeparator), "Add Separator");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_separator);
                },
                std::to_underlying(Menu::AddMenuSeparator));

            break;

        case gen_wxPropertyGridManager:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddPropgridPage), "Add Page");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_propGridPage);
                },
                std::to_underlying(Menu::AddPropgridPage));
            break;

        case gen_wxPropertyGrid:
        case gen_propGridPage:
        case gen_propGridCategory:
        case gen_propGridItem:
            add_sizer = false;
            if (!m_node->is_Gen(gen_propGridCategory) && !m_node->is_Gen(gen_propGridItem))
            {
                Append(std::to_underlying(Menu::AddPropgridCategory), "Add Category");
                Bind(
                    wxEVT_MENU,
                    [](wxCommandEvent&)
                    {
                        wxGetFrame().CreateToolNode(gen_propGridCategory);
                    },
                    std::to_underlying(Menu::AddPropgridCategory));
            }
            Append(std::to_underlying(Menu::AddPropgridItem), "Add Item");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_propGridItem);
                },
                std::to_underlying(Menu::AddPropgridItem));
            break;

        case gen_data_string:
        case gen_data_xml:
        case gen_data_folder:
            add_sizer = false;
            Append(std::to_underlying(Menu::AddDataString), "Add File");
            Append(std::to_underlying(Menu::AddDataXml), "Add XML File");
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_string);
                },
                std::to_underlying(Menu::AddDataString));
            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().CreateToolNode(gen_data_xml);
                },
                std::to_underlying(Menu::AddDataXml));
            break;

        default:
            if (m_is_parent_toolbar)
            {
                add_sizer = false;
                AddToolbarCommands(m_node);
            }
            else
            {
                Append(std::to_underlying(Menu::NewChildSpacer), "Add spacer");
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
    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();

    auto* sub_menu = new wxMenu;
    wxMenuItem* menu_item =
        sub_menu->Append(std::to_underlying(Menu::NewChildBoxSizer), "wxBoxSizer");
    menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildStaticSizer), "wxStaticBoxSizer");
    menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildWrapSizer), "wxWrapSizer");
    menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildGridSizer), "wxGridSizer");
    menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
    menu_item =
        sub_menu->Append(std::to_underlying(Menu::NewChildFlexGridSizer), "wxFlexGridSizer");
    menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildGridbagSizer), "wxGridBagSizer");
    menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

    if (child->is_Parent(gen_wxDialog))
    {
        sub_menu->AppendSeparator();
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewChildStdDialgBtns),
                                     "wxStdDialogButtonSizer");
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

    wxMenuItem* menu_item { nullptr };
    wxMenu* sub_menu { nullptr };
    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();
    AddSeparatorIfNeeded();

    BaseGenerator* generator = m_node->get_Generator();

    sub_menu = new wxMenu;
    if (m_parent && m_parent->is_Gen(GenEnum::gen_wxGridBagSizer))
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::MoveUp), "Decrease Row\tAlt+Up",
                                     "Decreases row containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_UP, wxART_MENU));
        menu_item = sub_menu->Append(std::to_underlying(Menu::MoveDown), "Increase Row\tAlt+Down",
                                     "Increases row containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_DOWN, wxART_MENU));

        menu_item =
            sub_menu->Append(std::to_underlying(Menu::MoveLeft), "Decrease Column\tAlt+Left",
                             "Decreases column containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_BACK, wxART_MENU));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::MoveRight), "Increase Column\tAlt+Right",
                             "Increases column containing selected item");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_FORWARD, wxART_MENU));
    }
    else
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::MoveUp), "Up\tAlt+Up",
                                     "Moves selected item up");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_UP, wxART_MENU));
        menu_item = sub_menu->Append(std::to_underlying(Menu::MoveDown), "Down\tAlt+Down",
                                     "Moves selected item down");
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_DOWN, wxART_MENU));

        if (!m_is_parent_toolbar && generator && generator->CanChangeParent(m_node))
        {
            menu_item = sub_menu->Append(std::to_underlying(Menu::MoveLeft), "Left\tAlt+Left",
                                         "Moves selected item left");
            menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_GO_BACK, wxART_MENU));
            menu_item = sub_menu->Append(std::to_underlying(Menu::MoveRight), "Right\tAlt+Right",
                                         "Moves selected item right");
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

    if (!m_is_parent_toolbar && generator && generator->CanChangeParent(m_node))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewParentBoxSizer), "wxBoxSizer");
        menu_item->SetBitmap(GetSvgImage("sizer_horizontal", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewParentStaticSizer), "wxStaticBoxSizer");
        menu_item->SetBitmap(GetSvgImage("wxStaticBoxSizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewParentWrapSizer), "wxWrapSizer");
        menu_item->SetBitmap(GetSvgImage("wrap_sizer", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::NewParentGridSizer), "wxGridSizer");
        menu_item->SetBitmap(GetSvgImage("grid_sizer", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewParentFlexGridSizer), "wxFlexGridSizer");
        menu_item->SetBitmap(GetSvgImage("flex_grid_sizer", dpi_size));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewParentGridbagSizer), "wxGridBagSizer");
        menu_item->SetBitmap(GetSvgImage("grid_bag_sizer", dpi_size));

        AppendSubMenu(sub_menu, "&Move into new sizer");
    }
    if (m_node->is_Form())
    {
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::NewParentFolder), "Move into new folder");
        menu_item->SetBitmap(GetInternalImage("folder"));
    }

    if (m_node->is_Gen(gen_wxRadioButton))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeTo2StateCheckbox),
                                     "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeTo3StateCheckbox),
                                     "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxCheckBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeTo3StateCheckbox),
                                     "3-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("check3state"));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToRadioButton), "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_Check3State))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeTo2StateCheckbox),
                                     "2-state wxCheckBox");
        menu_item->SetBitmap(GetInternalImage("wxCheckBox"));
        menu_item =
            sub_menu->Append(std::to_underlying(Menu::ChangeToRadioButton), "wxRadioButton");
        menu_item->SetBitmap(GetInternalImage("wxRadioButton"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxChoice))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToComboBox), "wxComboBox");
        menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBox), "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxComboBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBox), "wxChoice");
        menu_item->SetBitmap(GetSvgImage("wxChoice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBox), "wxListBox");
        menu_item->SetBitmap(GetInternalImage("wxListBox"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxListBox))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBox), "wxChoice");
        menu_item->SetBitmap(GetSvgImage("wxChoice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToComboBox), "wxComboBox");
        menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxAuiNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBook), "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBook), "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToNoteBook), "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToSimpleBook), "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxChoicebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToAuiBook), "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBook), "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToNoteBook), "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToSimpleBook), "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxListbook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToAuiBook), "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBook), "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToNoteBook), "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToSimpleBook), "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxNotebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToAuiBook), "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBook), "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBook), "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToSimpleBook), "wxSimplebook");
        menu_item->SetBitmap(GetInternalImage("wxSimplebook"));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    if (m_node->is_Gen(gen_wxSimplebook))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToAuiBook), "wxAuiNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook_aui", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToChoiceBook), "wxChoicebook");
        menu_item->SetBitmap(GetSvgImage("notebook_choice", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToListBook), "wxListbook");
        menu_item->SetBitmap(GetSvgImage("notebook_list", dpi_size));
        menu_item = sub_menu->Append(std::to_underlying(Menu::ChangeToNoteBook), "wxNotebook");
        menu_item->SetBitmap(GetSvgImage("notebook", dpi_size));
        AppendSubMenu(sub_menu, "&Change widget to");
    }
    return;
}

void NavPopupMenu::MenuAddStandardCommands()
{
    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();

    AddSeparatorIfNeeded();
    m_isPasteAllowed = false;
    if (m_node->is_Gen(gen_embedded_image))
    {
        wxMenuItem* menu_item = Append(wxID_DELETE);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));
        return;
    }
    if (m_node->is_Gen(gen_wxStatusBar))
    {
        wxMenuItem* menu_item = Append(wxID_CUT);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_CUT, wxART_MENU));

        menu_item = Append(wxID_COPY);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));

        menu_item = Append(wxID_DELETE);
        menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));
        return;
    }

    NodeSharedPtr clip_node = GetClipboardNode(false);
    if (!clip_node)
    {
        clip_node = wxGetFrame().getClipboardPtr();
    }
    m_isPasteAllowed = (clip_node ? true : false);

    if (m_node->is_Gen(gen_Project))
    {
        wxMenuItem* paste_menu_item = Append(wxID_PASTE);
        paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
        if (!clip_node || !clip_node->is_Form())
        {
            paste_menu_item->Enable(false);
            m_isPasteAllowed = false;
        }

        // There are no other standard commands for a project
        return;
    }

    wxMenuItem* menu_item { nullptr };
    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_CUT, wxART_MENU));

    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_COPY, wxART_MENU));

    wxMenuItem* paste_menu_item = Append(wxID_PASTE);
    paste_menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_PASTE, wxART_MENU));
    paste_menu_item->Enable((clip_node ? true : false));

    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_DELETE, wxART_MENU));

    AddSeparatorIfNeeded();
    if (!m_node->is_Gen(gen_Images) && !m_node->is_Gen(gen_Data))
    {
        menu_item = Append(std::to_underlying(Menu::Duplicate), "Duplicate");
        menu_item->SetBitmap(GetSvgImage("duplicate", dpi_size));
    }

    menu_item = Append(std::to_underlying(Menu::InsertWidget), "Insert Widget...");
    menu_item->SetBitmap(wxArtProvider::GetBitmapBundle(wxART_EDIT, wxART_MENU));
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent& event)
        {
            wxGetFrame().OnInsertWidget(event);
        },
        std::to_underlying(Menu::InsertWidget));
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

    const size_t childPos = m_parent->get_ChildPosition(m_node);

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
    {
        widget = "sub_folder";
    }

    // Avoid the temptation to set new_parent to the raw pointer so that .get() doesn't have to be
    // called below. Doing so will result in the reference count being decremented before we are
    // done hooking it up, and you end up crashing.

    const NodeSharedPtr new_parent = NodeCreation.CreateNode(widget, m_parent).first;
    if (new_parent)
    {
        wxGetFrame().Freeze();
        wxue::string undo_string("Insert new ");
        if (widget == "folder" || widget == "sub_folder")
        {
            undo_string << "folder";
        }
        else
        {
            undo_string << "sizer";
        }
        if (!m_parent->is_Gen(gen_wxGridBagSizer))
        {
            wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(
                new_parent.get(), m_parent, undo_string, childPos));
        }
        else
        {
            const NodeSharedPtr new_child = NodeCreation.MakeCopy(m_node);
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
    const wxWindowUpdateLocker freeze(wxGetFrame().getWindow());
    wxGetFrame().PushUndoAction(std::make_shared<ChangeSizerType>(m_node, new_sizer_gen));
}

void NavPopupMenu::ChangeNode(GenEnum::GenName new_node_gen)
{
    const wxWindowUpdateLocker freeze(wxGetFrame().getWindow());
    wxGetFrame().PushUndoAction(std::make_shared<ChangeNodeType>(m_node, new_node_gen));
}

void NavPopupMenu::AddToolbarCommands(Node* node)
{
    auto* sub_menu = new wxMenu;
    wxMenuItem* menu_item { nullptr };
    const wxSize& dpi_size = wxGetFrame().GetMenuDpiSize();

    AppendSubMenu(sub_menu, "Tools");

    const bool is_aui_toolbar =
        (node->get_GenName() == gen_wxAuiToolBar || node->get_GenName() == gen_AuiToolBar ||
         node->get_Parent()->get_GenName() == gen_wxAuiToolBar ||
         node->get_Parent()->get_GenName() == gen_AuiToolBar);

    menu_item = sub_menu->Append(std::to_underlying(Menu::AddTool), "Tool (normal, check, radio)");
    menu_item->SetBitmap(GetInternalImage("tool"));
    if (!is_aui_toolbar)
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolDropdown), "Dropdown");
        menu_item->SetBitmap(GetInternalImage("tool_dropdown"));
    }

    if (is_aui_toolbar)
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolLabel), "Label");
        menu_item->SetBitmap(GetSvgImage("wxStaticText", dpi_size));
    }

    sub_menu->AppendSeparator();
    menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolCombobox), "Combobox");
    menu_item->SetBitmap(GetSvgImage("wxComboBox", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolSlider), "Slider");
    menu_item->SetBitmap(GetSvgImage("slider", dpi_size));
    menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolSpinctrl), "Spin control");
    menu_item->SetBitmap(GetInternalImage("spin_ctrl"));
    sub_menu->AppendSeparator();

    menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolSeparator), "Separator");
    menu_item->SetBitmap(GetInternalImage("toolseparator"));
    if (!is_aui_toolbar)
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolStretchableSpace),
                                     "Stretchable space");
        menu_item->SetBitmap(GetInternalImage("toolStretchable"));
    }
    else
    {
        menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolSpacer), "Spacer");
        menu_item->SetBitmap(GetInternalImage("toolspacer"));
        menu_item = sub_menu->Append(std::to_underlying(Menu::AddToolStretchableSpacer),
                                     "Stretchable spacer");
        menu_item->SetBitmap(GetInternalImage("toolStretchable"));
    }

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxComboBox);
        },
        std::to_underlying(Menu::AddToolCombobox));
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxSlider);
        },
        std::to_underlying(Menu::AddToolSlider));
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxSpinCtrl);
        },
        std::to_underlying(Menu::AddToolSpinctrl));
    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_toolSeparator);
        },
        std::to_underlying(Menu::AddToolSeparator));

    if (!is_aui_toolbar)
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_tool);
            },
            std::to_underlying(Menu::AddTool));
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_tool_dropdown);
            },
            std::to_underlying(Menu::AddToolDropdown));
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_toolStretchable);
            },
            std::to_underlying(Menu::AddToolStretchableSpace));
    }
    else
    {
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool);
            },
            std::to_underlying(Menu::AddTool));
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_label);
            },
            std::to_underlying(Menu::AddToolLabel));
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_spacer);
            },
            std::to_underlying(Menu::AddToolSpacer));
        Bind(
            wxEVT_MENU,
            [](wxCommandEvent&)
            {
                wxGetFrame().CreateToolNode(gen_auitool_stretchable);
            },
            std::to_underlying(Menu::AddToolStretchableSpacer));
    }
}

void NavPopupMenu::AddSeparatorIfNeeded()
{
    if (const size_t count = GetMenuItemCount(); count)
    {
        const wxMenuItem* menu_item = FindItemByPosition(count - 1);
        if (menu_item->GetKind() != wxITEM_SEPARATOR)
        {
            AppendSeparator();
        }
    }
}
