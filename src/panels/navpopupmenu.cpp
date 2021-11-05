/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/artprov.h>

#include "navpopupmenu.h"  // NavPopupMenu

#include "bitmaps.h"         // Contains various images handling functions
#include "mainframe.h"       // MainFrame -- Main window frame
#include "mainframe_base.h"  // contains all the wxue_img namespace embedded images
#include "nav_panel.h"       // NavigationPanel -- Navigation Panel
#include "node.h"            // Node class
#include "node_creator.h"    // NodeCreator class
#include "uifuncs.h"         // Miscellaneous functions for displaying UI
#include "undo_cmds.h"       // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"           // Utility functions that work with properties

#include "newdialog.h"  // NewDialog -- Dialog for creating a new project dialog
#include "newframe.h"   // NewFrame -- Dialog for creating a new project wxFrame

// clang-format off
static const auto lstBarGenerators = {

    gen_wxStatusBar,
    gen_wxMenuBar,
    gen_wxToolBar,

    gen_MenuBar,  // form version of wxMenuBar
    gen_RibbonBar,  // form version of wxRibbonBar
    gen_ToolBar,  // form version of wxToolBar

    gen_wxRibbonBar,
    gen_wxRibbonPage,
    gen_wxRibbonToolBar,
    gen_ribbonTool,

};

static const auto lstContainerGenerators = {

    gen_BookPage,
    gen_wxPanel,
    gen_wxWizardPageSimple,

};
// clang-format on

NavPopupMenu::NavPopupMenu(Node* node) : m_node(node)
{
    if (!node)
    {
        ASSERT(node);
        return;  // theoretically impossible, but don't crash if it happens
    }

    for (auto& iter: lstBarGenerators)
    {
        if (node->isGen(iter))
        {
            CreateBarMenu(node);
            return;
        }
    }

    if (node->isGen(gen_PopupMenu))
    {
        CreateMenuMenu(node);
        return;
    }

    if (node->IsForm())
    {
        CreateFormMenu(node);
        return;
    }

    for (auto& iter: lstContainerGenerators)
    {
        if (node->isGen(iter))
        {
            CreateContainerMenu(node);
            return;
        }
    }

    if (node->IsContainer() && node->DeclName().contains("book", tt::CASE::either))
    {
        CreateBookMenu(node);
        return;
    }

    if (node->isType(type_project))
    {
        CreateProjectMenu(node);
        return;
    }

    if (node->DeclName().is_sameprefix("wxMenu") || node->isGen(gen_separator) || node->isGen(gen_submenu))
    {
        CreateMenuMenu(node);
        return;
    }

    if (node->isGen(gen_wxWizard))
    {
        CreateWizardMenu(node);
        return;
    }

    if (node->IsSizer())
    {
        if (node->GetParent()->IsForm() || node->GetParent()->IsContainer())
        {
            CreateTopSizerMenu(node);
        }
        else
        {
            CreateSizerMenu(node);
        }
        return;
    }

    CreateNormalMenu(node);
}

void NavPopupMenu::OnMenuEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MenuNEW_ITEM:
            if (m_tool_name < gen_name_array_size)
            {
                if (m_child)
                    m_child->CreateToolNode(m_tool_name);
                else
                    wxGetFrame().CreateToolNode(m_tool_name);
            }
            break;

        case MenuEXPAND_ALL:
            wxGetFrame().GetNavigationPanel()->ExpandCollapse(m_node);
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

        case MenuChangeTo_FLEX_GRID_SIZER:
            ChangeSizer(gen_wxFlexGridSizer);
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

        case MenuRESET_ID:
            m_node->ModifyProperty("id", "wxID_ANY");
            break;

        case MenuRESET_SIZE:
            m_node->ModifyProperty("size", "-1,-1");
            break;

        case MenuRESET_MIN_SIZE:
            m_node->ModifyProperty(prop_minimum_size, "-1,-1");
            break;

        case MenuRESET_MAX_SIZE:
            m_node->ModifyProperty("maximum_size", "-1,-1");
            break;

        case MenuADD_PAGE:
            if (m_node->isGen(gen_BookPage))
            {
                m_node->GetParent()->CreateToolNode(gen_BookPage);
            }
            if (m_node->isGen(gen_wxWizardPageSimple))
            {
                m_node->GetParent()->CreateToolNode(gen_wxWizardPageSimple);
            }
            else
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
            if (m_node && !m_node->isGen(gen_Project))
                event.Enable(true);
            else
                event.Enable(wxGetFrame().CanCopyNode());
            break;

        case wxID_PASTE:
            event.Enable(wxGetFrame().CanPasteNode());
            break;

        case MenuBORDERS_ALL:
            event.Check(m_node->prop_as_string(prop_borders).contains("wxALL"));
            break;

        case MenuBORDERS_NONE:
            event.Check(m_node->prop_as_string(prop_borders).empty());
            break;

        case MenuBORDERS_HORIZONTAL:
            event.Check(m_node->prop_as_string(prop_borders) == "wxLEFT|wxRIGHT" ||
                        m_node->prop_as_string(prop_borders) == "wxRIGHT|wxLEFT|");
            break;

        case MenuBORDERS_VERTICAL:
            event.Check(m_node->prop_as_string(prop_borders) == "wxTOP|wxBOTTOM" ||
                        m_node->prop_as_string(prop_borders) == "wxBOTTOM|wxTOP|");
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

void NavPopupMenu::CreateSizerParent(Node* node, ttlib::cview widget)
{
    auto parent = node->GetParent();
    if (!parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user know why.")
#if !defined(_DEBUG)
        appMsgBox(ttlib::cstr("An internal error occurred. The following node is missing a parent: ")
                      << node->get_node_name(),
                  "CreateSizerParent()");
        throw;
#else
        return;
#endif  // _DEBUG
    }

    auto childPos = parent->GetChildPosition(node);

    while (parent && !parent->IsSizer())
    {
        parent = parent->GetParent();
    }

    if (!parent)
    {
        // If this actually happens, then we silently do nothing leaving the user no idea of why it didn't work
        FAIL_MSG("If this occurs, we need to figure out why and then add a message to let the user know why.")
#if !defined(_DEBUG)
        appMsgBox(ttlib::cstr("An internal error occurred creating a sizer parent for ") << node->get_node_name(),
                  "CreateSizerParent()");
        throw;
#else
        return;
#endif  // _DEBUG
    }

    // Avoid the temptation to set new_sizer to the raw pointer so that .get() doesn't have to be called below. Doing so will
    // result in the reference count being decremented before we are done hooking it up, and you end up crashing.

    auto new_sizer = g_NodeCreator.CreateNode(widget, parent);
    if (new_sizer)
    {
        wxGetFrame().Freeze();
        wxGetFrame().PushUndoAction(
            std::make_shared<InsertNodeAction>(new_sizer.get(), parent, "Insert new sizer", childPos));

        // InsertNodeAction does not fire the creation event since that's usually handled by the caller as needed. We don't
        // want to fire an event because we don't want the Mockup or Code panels to update until we have changed the parent.
        // However we *do* need to let the navigation panel know that a new node has been added.

        wxGetFrame().GetNavigationPanel()->InsertNode(new_sizer.get());

        wxGetFrame().PushUndoAction(std::make_shared<ChangeParentAction>(node, new_sizer.get()));
        wxGetFrame().SelectNode(node, true, true);
        wxGetFrame().Thaw();
    }
}

void NavPopupMenu::ChangeSizer(GenEnum::GenName new_sizer_gen)
{
    wxGetFrame().Freeze();
    wxGetFrame().PushUndoAction(std::make_shared<ChangeSizerType>(m_node, new_sizer_gen));
    wxGetFrame().Thaw();
}

void NavPopupMenu::OnBorders(wxCommandEvent& event)
{
    ttlib::cstr value;

    switch (event.GetId())
    {
        case MenuBORDERS_ALL:
            value = "wxALL";
            break;

        case MenuBORDERS_NONE:
            // It's already cleared, so nothing to do
            break;

        case MenuBORDERS_HORIZONTAL:
            value = "wxLEFT|wxRIGHT";
            break;

        case MenuBORDERS_VERTICAL:
            value = "wxTOP|wxBOTTOM";
            break;
    }

    m_node->ModifyProperty(prop_borders, value);
}

void NavPopupMenu::CreateNormalMenu(Node* node)
{
    wxMenuItem* menu_item;
    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));
    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));

    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_MENU));
    Append(MenuDUPLICATE, "Duplicate");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));
    menu_item = sub_menu->Append(MenuMOVE_LEFT, "Left\tAlt+Left", "Moves selected item left");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveleft_png, sizeof(wxue_img::nav_moveleft_png)));
    menu_item = sub_menu->Append(MenuMOVE_RIGHT, "Right\tAlt+Right", "Moves selected item right");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveleft_png, sizeof(wxue_img::nav_moveleft_png)));
    AppendSubMenu(sub_menu, "Move");

    sub_menu = new wxMenu;
    sub_menu->Append(MenuRESET_ID, "ID", "Changes id to wxANY");
    sub_menu->Append(MenuRESET_SIZE, "Size", "Changes size to -1, -1");
    sub_menu->Append(MenuRESET_MIN_SIZE, "Minimum size", "Changes minimum size to -1, -1");
    sub_menu->Append(MenuRESET_MAX_SIZE, "Maximum size", "Changes maximum size to -1, -1");
    AppendSubMenu(sub_menu, "Reset");

    sub_menu = new wxMenu;
    sub_menu->Append(MenuBORDERS_ALL, "All", "Borders on all sides", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_NONE, "None", "No borders", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_HORIZONTAL, "Horizontal only", "Borders only on left and right", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_VERTICAL, "Vertical only", "Borders only on top and bottom", wxITEM_CHECK);
    AppendSubMenu(sub_menu, "Borders");

    AppendSeparator();
    if (node->gen_type() == type_sizer)
    {
#if 0
// See TODO comment in OnAddNew below
            Append(MenuNEW_SIBLING_SPACER, "Add spacer");
#endif
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

        AppendSubMenu(sub_menu, "Add child sizer");

        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

        AppendSubMenu(sub_menu, "Add sibling sizer");
    }
    else
    {
        Append(MenuNEW_CHILD_SPACER, "Add spacer");

        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

        AppendSubMenu(sub_menu, "Add sizer");
    }

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_PARENT_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_PARENT_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_PARENT_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_PARENT_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetInternalImage("grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_PARENT_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_PARENT_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

    AppendSubMenu(sub_menu, "&Move into new sizer");

    if (node->DeclName().contains("book"))
    {
        AppendSeparator();
        Append(MenuADD_PAGE, "Add page");
    }

    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);

    // Bind() uses the equivalent of a LIFO stack, so we can Bind() to wxID_ANY first, then Bind() to any specific ids as
    // needed

    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_ALL);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_NONE);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_HORIZONTAL);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_VERTICAL);

    // The OnAddNew commands add to a child, so we need to "fake" the child to our parent in order to add a sibling or a
    // child
    m_child = node->GetParent();

    if (node->gen_type() == type_sizer)
    {
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_BOX_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_STATIC_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_WRAP_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_FLEX_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRIDBAG_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_SPACER);
    }
    else
    {
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_SPACER);
    }

    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_BOX_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_STATIC_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_WRAP_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_FLEX_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRIDBAG_SIZER);
}

void NavPopupMenu::CreateProjectMenu(Node* node)
{
    if (wxGetFrame().GetClipboard())
    {
        auto clipboard = wxGetFrame().GetClipboard();

        // The selected node is a container, so there aren't very many things you can paste into it.

        if (clipboard->IsForm() || clipboard->IsContainer() || (clipboard->IsSizer() && node->GetChildCount() == 0))
        {
            auto menu_item = Append(wxID_PASTE);
            menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
            AppendSeparator();

            Bind(
                wxEVT_MENU,
                [](wxCommandEvent&)
                {
                    wxGetFrame().PasteNode(wxGetApp().GetProject());
                    ;
                },
                wxID_PASTE);
        }
    }

    Append(MenuPROJECT_ADD_DIALOG, "Add new dialog...");
    Append(MenuPROJECT_ADD_WINDOW, "Add new window...");
    Append(MenuPROJECT_ADD_WIZARD, "Add new wizard");
    AppendSeparator();
    Append(MenuPROJECT_SORT_FORMS, "Sort Forms");

    Bind(wxEVT_MENU, &NavPopupMenu::OnCreateNewDialog, this, MenuPROJECT_ADD_DIALOG);
    Bind(wxEVT_MENU, &NavPopupMenu::OnCreateNewFrame, this, MenuPROJECT_ADD_WINDOW);
    Bind(wxEVT_MENU, &NavPopupMenu::OnSortForms, this, MenuPROJECT_SORT_FORMS);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxWizard);
            ;
        },
        MenuPROJECT_ADD_WIZARD);
}

void NavPopupMenu::OnAddNew(wxCommandEvent& event)
{
    if (m_tool_name < gen_name_array_size)
    {
        if (m_child)
            m_child->CreateToolNode(m_tool_name);
        else
            wxGetFrame().CreateToolNode(m_tool_name);
        return;
    }

    switch (event.GetId())
    {
        case MenuNEW_SIBLING_BOX_SIZER:
            m_child->CreateToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_SIBLING_STATIC_SIZER:
            m_child->CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_SIBLING_WRAP_SIZER:
            m_child->CreateToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_SIBLING_GRID_SIZER:
            m_child->CreateToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_SIBLING_FLEX_GRID_SIZER:
            m_child->CreateToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_SIBLING_GRIDBAG_SIZER:
            m_child->CreateToolNode(gen_wxGridBagSizer);
            break;

        case MenuNEW_SIBLING_STD_DIALG_BTNS:
            m_child->CreateToolNode(gen_wxStdDialogButtonSizer);
            break;

#if 0
        // TODO: [KeyWorks - 08-17-2020] This should work the same way as adding a sibling when a sizer is selected, but what
        // actually happens is that the spacer is added above rather than below the sizer. All other sizers get added below, so
        // need to figure out why before this gets enabled.

        case MenuNEW_SIBLING_SPACER:
            m_child->CreateToolNode("spacer");
            break;
#endif

        case MenuNEW_CHILD_BOX_SIZER:
            wxGetFrame().CreateToolNode(gen_wxBoxSizer);
            break;

        case MenuNEW_CHILD_STATIC_SIZER:
            wxGetFrame().CreateToolNode(gen_wxStaticBoxSizer);
            break;

        case MenuNEW_CHILD_WRAP_SIZER:
            wxGetFrame().CreateToolNode(gen_wxWrapSizer);
            break;

        case MenuNEW_CHILD_GRID_SIZER:
            wxGetFrame().CreateToolNode(gen_wxGridSizer);
            break;

        case MenuNEW_CHILD_FLEX_GRID_SIZER:
            wxGetFrame().CreateToolNode(gen_wxFlexGridSizer);
            break;

        case MenuNEW_CHILD_GRIDBAG_SIZER:
            wxGetFrame().CreateToolNode(gen_wxGridBagSizer);
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

        default:
            break;
    }
}

void NavPopupMenu::CreateContainerMenu(Node* node)
{
    wxMenuItem* menu_item;

    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));
    if (wxGetFrame().GetClipboard())
    {
        auto clipboard = wxGetFrame().GetClipboard();

        // The selected node is a container, so there aren't very many things you can paste into it.

        if (clipboard->IsForm() || clipboard->IsContainer() || (clipboard->IsSizer() && node->GetChildCount() == 0))
        {
            menu_item = Append(wxID_PASTE);
            menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
        }
    }

    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_MENU));
    AppendSeparator();

    auto sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));
    AppendSubMenu(sub_menu, "Move");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    if (node->isGen(gen_BookPage))
    {
        if (!node->GetChildCount())
        {
            sub_menu = new wxMenu;
            menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
            menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
            menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
            menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
            menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
            menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
            menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
            menu_item->SetBitmap(GetInternalImage("grid_sizer"));
            menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
            menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
            menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
            menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));
            AppendSubMenu(sub_menu, "Add sizer");
        }
        Append(MenuADD_PAGE, "Add Page\tCtrl+P");
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_ITEM);

        if (!node->GetChildCount())
        {
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_BOX_SIZER);
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_STATIC_SIZER);
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_WRAP_SIZER);
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRID_SIZER);
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_FLEX_GRID_SIZER);
            Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRIDBAG_SIZER);
        }
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_ITEM);
    }
    else if (node->GetChildCount())
    {
        m_child = node->GetChild(0);

        AppendSeparator();
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));
        if (node->isGen(gen_wxDialog))
        {
            sub_menu->AppendSeparator();
            menu_item = sub_menu->Append(MenuNEW_SIBLING_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
            menu_item->SetBitmap(GetInternalImage("stddialogbuttonsizer"));
        }
        AppendSubMenu(sub_menu, "&Add new sizer");

        if (node->isGen(gen_wxWizardPageSimple))
        {
            Append(MenuADD_PAGE, "Add Page\tCtrl+P");
            Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, MenuADD_PAGE);
        }

        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_BOX_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_STATIC_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_WRAP_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_FLEX_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRIDBAG_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_STD_DIALG_BTNS);
    }

    else if (node->isGen(gen_wxWizardPageSimple))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
        menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));
        AppendSubMenu(sub_menu, "Add sizer");

        Append(MenuADD_PAGE, "Add Page\tCtrl+P");

        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_BOX_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_STATIC_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_WRAP_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_FLEX_GRID_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRIDBAG_SIZER);
        Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, MenuADD_PAGE);
    }
}

void NavPopupMenu::CreateFormMenu(Node* node)
{
    m_node = node;

    wxMenuItem* menu_item;

    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));

    AppendSeparator();
    menu_item = Append(MenuEXPAND_ALL, "Expand All");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
}

void NavPopupMenu::CreateTopSizerMenu(Node* node)
{
    wxMenuItem* menu_item;

    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
    AppendSeparator();

    // Many of the OnAddNew commands add to a child, so we need to "fake" the child to ourselves
    m_child = node;

    auto sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetInternalImage("grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

    if (node->isParent(gen_wxDialog))
    {
        sub_menu->AppendSeparator();
        menu_item = sub_menu->Append(MenuNEW_SIBLING_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
        menu_item->SetBitmap(GetInternalImage("stddialogbuttonsizer"));
    }
    AppendSubMenu(sub_menu, "&Add new sizer");

    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_PASTE);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
}

void NavPopupMenu::CreateSizerMenu(Node* node)
{
    wxMenuItem* menu_item;
    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));
    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));

    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_MENU));
    Append(MenuDUPLICATE, "Duplicate");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));
    menu_item = sub_menu->Append(MenuMOVE_LEFT, "Left\tAlt+Left", "Moves selected item left");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveleft_png, sizeof(wxue_img::nav_moveleft_png)));
    menu_item = sub_menu->Append(MenuMOVE_RIGHT, "Right\tAlt+Right", "Moves selected item right");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveleft_png, sizeof(wxue_img::nav_moveleft_png)));
    AppendSubMenu(sub_menu, "Move");

    AppendSeparator();
    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetInternalImage("grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

    AppendSubMenu(sub_menu, "Add child sizer");

    sub_menu = new wxMenu;
    menu_item = sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
    menu_item->SetBitmap(GetInternalImage("sizer_horizontal"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
    menu_item->SetBitmap(GetInternalImage("wxStaticBoxSizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
    menu_item->SetBitmap(GetInternalImage("wrap_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
    menu_item->SetBitmap(GetInternalImage("grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
    menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
    menu_item = sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
    menu_item->SetBitmap(GetInternalImage("grid_bag_sizer"));

    // The OnAddNew commands add to a child, so we need to "fake" the child to our parent in order to add a sibling or a
    // child
    m_child = node->GetParent();

    AppendSubMenu(sub_menu, "Add sibling sizer");

    ASSERT_MSG(!node->GetParent()->IsForm() && !node->GetParent()->IsContainer(),
               "This popup menu should never be called if parent is a form or container!");

    if (node->isGen(gen_wxBoxSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
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
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    else if (node->isGen(gen_wxFlexGridSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));
        menu_item = sub_menu->Append(MenuChangeTo_WRAP_SIZER, "wxWrapSizer");
        menu_item->SetBitmap(GetInternalImage("wrap_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }
    else if (node->isGen(gen_wxWrapSizer))
    {
        sub_menu = new wxMenu;
        menu_item = sub_menu->Append(MenuChangeTo_FLEX_GRID_SIZER, "wxFlexGridSizer");
        menu_item->SetBitmap(GetInternalImage("flex_grid_sizer"));
        menu_item = sub_menu->Append(MenuChangeTo_GRID_SIZER, "wxGridSizer");
        menu_item->SetBitmap(GetInternalImage("grid_sizer"));

        AppendSubMenu(sub_menu, "Change Sizer To");
    }

    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);

    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_BOX_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_STATIC_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_WRAP_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_FLEX_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_GRIDBAG_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_SIBLING_SPACER);

    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_BOX_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_STATIC_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_WRAP_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_FLEX_GRID_SIZER);
    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuNEW_CHILD_GRIDBAG_SIZER);
}

void NavPopupMenu::CreateMenuMenu(Node* /* node */)
{
    wxMenuItem* menu_item;

    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));
    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
    menu_item = Append(wxID_DELETE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_DELETE, wxART_MENU));

    AppendSeparator();
    menu_item = Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));

    AppendSeparator();
    Append(MenuADD_MENUITEM, "Add Menu &Item\tCtrl+I");
    Append(MenuADD_SUBMENU, "Add &Submenu &Item\tCtrl+S");
    Append(MenuADD_SEPARATOR, "Add S&eparator\tCtrl+E");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_wxMenuItem);
            ;
            ;
        },
        MenuADD_MENUITEM);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_submenu);
            ;
        },
        MenuADD_SUBMENU);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_separator);
            ;
        },
        MenuADD_SEPARATOR);
}

void NavPopupMenu::CreateBarMenu(Node* node)
{
    m_node = node;

    wxMenuItem* menu_item;

    menu_item = Append(wxID_CUT);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_CUT, wxART_MENU));
    menu_item = Append(wxID_COPY);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_COPY, wxART_MENU));

    AppendSeparator();
    menu_item = Append(MenuEXPAND_ALL, "Expand All");

    if (!node->isGen(gen_wxStatusBar))
    {
        menu_item = Append(wxID_PASTE);
        menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
        AppendSeparator();
    }

    if (node->isGen(gen_wxMenuBar) || node->isGen(gen_MenuBar))
    {
        AppendSeparator();
        Append(MenuADD_MENU, "Add Menu\tCtrl+M");
    }
    else if (node->isGen(gen_wxRibbonBar) || node->isGen(gen_RibbonBar))
    {
        AppendSeparator();
        m_tool_name = gen_wxRibbonPage;
        Append(MenuNEW_ITEM, "Add Page");
    }
    else if (node->isGen(gen_wxRibbonPage))
    {
        AppendSeparator();
        m_tool_name = gen_wxRibbonPanel;
        Append(MenuNEW_ITEM, "Add Panel");
    }
    else if (node->isGen(gen_wxRibbonToolBar))
    {
        AppendSeparator();
        m_tool_name = gen_ribbonTool;
        Append(MenuNEW_ITEM, "Add Tool");
    }
    else if (node->isGen(gen_ribbonTool))
    {
        m_child = node->GetParent();
        AppendSeparator();
        m_tool_name = gen_ribbonTool;
        Append(MenuNEW_ITEM, "Add Tool");
    }

    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);

    if (node->isGen(gen_wxMenuBar) || node->isGen(gen_MenuBar))
    {
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuADD_MENU);
    }
}

void NavPopupMenu::CreateWizardMenu(Node* /* node */)
{
    wxMenuItem* menu_item;

    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
    AppendSeparator();
    menu_item = Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));

    AppendSeparator();
    m_tool_name = gen_wxWizardPageSimple;
    Append(MenuNEW_ITEM, "Add Page\tCtrl+P");

    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
}

void NavPopupMenu::CreateBookMenu(Node* /* node */)
{
    wxMenuItem* menu_item;

    menu_item = Append(wxID_PASTE);
    menu_item->SetBitmap(wxArtProvider::GetBitmap(wxART_PASTE, wxART_MENU));
    AppendSeparator();
    menu_item = Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_moveup_png, sizeof(wxue_img::nav_moveup_png)));
    menu_item = Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    menu_item->SetBitmap(GetImageFromArray(wxue_img::nav_movedown_png, sizeof(wxue_img::nav_movedown_png)));

    AppendSeparator();
    m_tool_name = gen_BookPage;
    Append(MenuNEW_ITEM, "Add Page");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    sub_menu->Append(MenuBORDERS_ALL, "All", "Borders on all sides", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_NONE, "None", "No borders", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_HORIZONTAL, "Horizontal only", "Borders only on left and right", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_VERTICAL, "Vertical only", "Borders only on top and bottom", wxITEM_CHECK);
    AppendSubMenu(sub_menu, "Borders");

    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
}

void NavPopupMenu::OnCreateNewDialog(wxCommandEvent& WXUNUSED(event))
{
    NewDialog dlg;
    if (dlg.ShowModal() == wxID_OK)
    {
        dlg.CreateNode();
    }
}

void NavPopupMenu::OnCreateNewFrame(wxCommandEvent& WXUNUSED(event))
{
    NewFrame dlg;
    if (dlg.ShowModal() == wxID_OK)
    {
        dlg.CreateNode();
    }
}

void NavPopupMenu::OnSortForms(wxCommandEvent& WXUNUSED(event))
{
    wxGetFrame().PushUndoAction(std::make_shared<SortProjectAction>());
}
