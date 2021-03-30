/////////////////////////////////////////////////////////////////////////////
// Purpose:   Context-menu for Navigation Panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "navpopupmenu.h"  // NavPopupMenu

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator class
#include "uifuncs.h"       // Miscellaneous functions for displaying UI
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction
#include "utils.h"         // Utility functions that work with properties

// clang-format off
static const auto lstBarClasses = {

    "wxStatusBar",
    "wxMenuBar",
    "wxToolBar",

    "MenuBar",  // form version of wxMenuBar
    "ToolBar",  // form version of wxToolBar

    "wxRibbonBar",
    "wxRibbonPage",
    "wxRibbonToolBar",
    "ribbonTool",

};

static const auto lstContainerClasses = {

    "wxDialog",
    "wxPanel",
    "PanelForm",
    "BookPage",
    "wxWizardPageSimple",

};
// clang-format on

NavPopupMenu::NavPopupMenu(Node* node) : m_node(node)
{
    if (!node)
    {
        ASSERT(node);
        return;  // theoretically impossible, but don't crash if it happens
    }

    for (auto& iter: lstBarClasses)
    {
        if (node->GetClassName() == iter)
        {
            CreateBarMenu(node);
            return;
        }
    }

    for (auto& iter: lstContainerClasses)
    {
        if (node->GetClassName() == iter)
        {
            CreateContainerMenu(node);
            return;
        }
    }

    if (node->IsContainer() && node->GetClassName().contains("book"))
    {
        CreateBookMenu(node);
        return;
    }

    if (node->GetNodeTypeName() == "project")
    {
        CreateProjectMenu(node);
        return;
    }
    else if (ttlib::is_sameprefix(node->GetClassName(), "wxMenu") || node->GetClassName() == "separator" ||
             node->GetClassName() == "submenu")
    {
        CreateMenuMenu(node);
        return;
    }
    else if (node->GetClassName() == "wxWizard")
    {
        CreateWizardMenu(node);
        return;
    }

    auto parent = node->GetParent();

    if (node->IsSizer() && (parent->IsForm() || parent->IsContainer()))
    {
        CreateTopSizerMenu(node);
        return;
    }

    Append(MenuCUT, "Cut\tCtrl+X");
    Append(MenuCOPY, "Copy\tCtrl+C");
    Append(MenuPASTE, "Paste\tCtrl+V");
    Append(MenuDELETE, "Delete\tCtrl+D");
    Append(MenuDUPLICATE, "Duplicate");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    sub_menu->Append(MenuMOVE_LEFT, "Left\tAlt+Left", "Moves selected item left");
    sub_menu->Append(MenuMOVE_RIGHT, "Right\tAlt+Right", "Moves selected item right");
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
    if (node->GetNodeTypeName() == "sizer")
    {
#if 0
// See TODO comment in OnAddNew below
            Append(MenuNEW_SIBLING_SPACER, "Add spacer");
#endif
        sub_menu = new wxMenu;
        sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
        sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");

        AppendSubMenu(sub_menu, "Add child sizer");

        sub_menu = new wxMenu;
        sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
        sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");

        AppendSubMenu(sub_menu, "Add sibling sizer");
    }
    else
    {
        Append(MenuNEW_CHILD_SPACER, "Add spacer");

        sub_menu = new wxMenu;
        sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
        sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");

        AppendSubMenu(sub_menu, "Add sizer");
    }

    sub_menu = new wxMenu;
    sub_menu->Append(MenuNEW_PARENT_BOX_SIZER, "wxBoxSizer");
    sub_menu->Append(MenuNEW_PARENT_STATIC_SIZER, "wxStaticBoxSizer");
    sub_menu->Append(MenuNEW_PARENT_WRAP_SIZER, "wxWrapSizer");
    sub_menu->Append(MenuNEW_PARENT_GRID_SIZER, "wxGridSizer");
    sub_menu->Append(MenuNEW_PARENT_FLEX_GRID_SIZER, "wxFlexGridSizer");
    sub_menu->Append(MenuNEW_PARENT_GRIDBAG_SIZER, "wxGridBagSizer");

    AppendSubMenu(sub_menu, "&Move into new sizer");

    if (ttlib::contains(node->GetClassName(), "book"))
    {
        AppendSeparator();
        Append(MenuADD_PAGE, "Add page");
    }

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    // Bind() uses the equivalent of a LIFO stack, so we can Bind() to wxID_ANY first, then Bind() to any specific ids as
    // needed

    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_ALL);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_NONE);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_HORIZONTAL);
    Bind(wxEVT_MENU, &NavPopupMenu::OnBorders, this, MenuBORDERS_VERTICAL);

    // The OnAddNew commands add to a child, so we need to "fake" the child to our parent in order to add a sibling or a
    // child
    m_child = node->GetParent();

    if (node->GetNodeTypeName() == "sizer")
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

    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(m_node, MoveDirection::Up, true));
    Enable(MenuMOVE_DOWN, wxGetFrame().MoveNode(m_node, MoveDirection::Down, true));
    Enable(MenuMOVE_LEFT, wxGetFrame().MoveNode(m_node, MoveDirection::Left, true));
    Enable(MenuMOVE_RIGHT, wxGetFrame().MoveNode(m_node, MoveDirection::Right, true));
}

void NavPopupMenu::OnMenuEvent(wxCommandEvent& event)
{
    switch (event.GetId())
    {
        case MenuNEW_ITEM:
            if (m_tool_name.size())
            {
                if (m_child)
                    m_child->CreateToolNode(m_tool_name);
                else
                    wxGetFrame().CreateToolNode(m_tool_name);
            }
            break;

        case MenuCUT:
            wxGetFrame().CutNode(m_node);
            break;

        case MenuCOPY:
            wxGetFrame().CopyNode(m_node);
            break;

        case MenuPASTE:
            wxGetFrame().PasteNode(m_node);
            break;

        case MenuDUPLICATE:
            wxGetFrame().DuplicateNode(m_node);
            break;

        case MenuDELETE:
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

        case MenuRESET_ID:
            m_node->ModifyProperty("id", "wxID_ANY");
            break;

        case MenuRESET_SIZE:
            m_node->ModifyProperty("size", "-1,-1");
            break;

        case MenuRESET_MIN_SIZE:
            m_node->ModifyProperty(txt_minimum_size, "-1,-1");
            break;

        case MenuRESET_MAX_SIZE:
            m_node->ModifyProperty("maximum_size", "-1,-1");
            break;

        case MenuADD_PAGE:
            if (m_node->GetClassName() == "BookPage")
            {
                m_node->GetParent()->CreateToolNode("BookPage");
            }
            if (m_node->GetClassName() == "wxWizardPageSimple")
            {
                m_node->GetParent()->CreateToolNode("wxWizardPageSimple");
            }
            else
            {
                wxGetFrame().CreateToolNode("wxPanel");
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
        case MenuCUT:
        case MenuCOPY:
        case MenuDELETE:
            event.Enable(wxGetFrame().CanCopyNode());
            break;

        case MenuPASTE:
            event.Enable(wxGetFrame().CanPasteNode());
            break;

        case MenuBORDERS_ALL:
            event.Check(m_node->prop_as_string(txt_borders).contains("wxALL"));
            break;

        case MenuBORDERS_NONE:
            event.Check(m_node->prop_as_string(txt_borders).empty());
            break;

        case MenuBORDERS_HORIZONTAL:
            event.Check(m_node->prop_as_string(txt_borders) == "wxLEFT|wxRIGHT" ||
                        m_node->prop_as_string(txt_borders) == "wxRIGHT|wxLEFT|");
            break;

        case MenuBORDERS_VERTICAL:
            event.Check(m_node->prop_as_string(txt_borders) == "wxTOP|wxBOTTOM" ||
                        m_node->prop_as_string(txt_borders) == "wxBOTTOM|wxTOP|");
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
        return;
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
        return;
    }

    // Avoid the temptation to set new_sizer to the raw pointer so that .get() doesn't have to be called below. Doing so will
    // result in the reference count being decremented before we are done hooking it up, and you end up crashing (see issue
    // #93).

    auto new_sizer = g_NodeCreator.CreateNode(widget, parent);
    if (new_sizer)
    {
        auto multi_cmd = std::make_shared<MultiAction>(ttlib::cstr() << "new sizer for " << node->GetClassName());

        auto reparent_cmd = std::make_shared<ChangeParentAction>(node, new_sizer.get());
        multi_cmd->Add(reparent_cmd);

        auto insert_cmd = std::make_shared<InsertNodeAction>(new_sizer.get(), parent, tt_empty_cstr, childPos);
        multi_cmd->Add(insert_cmd);

        wxGetFrame().PushUndoAction(multi_cmd);
        new_sizer->FixDuplicateName();

        // REVIEW: [KeyWorks - 03-30-2021] See issue #94 about the problem this causes.
        wxGetFrame().FireProjectUpdatedEvent();

        wxGetFrame().SelectNode(new_sizer->GetChild(0), true, true);
    }
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

    m_node->ModifyProperty(txt_borders, value);
}

void NavPopupMenu::CreateProjectMenu(Node* WXUNUSED(node))
{
    Append(MenuPROJECT_ADD_DIALOG, "Add new dialog");
    Append(MenuPROJECT_ADD_TABBED_DIALOG, "Add new tabbed dialog");
    Append(MenuPROJECT_ADD_WINDOW, "Add new window");
    Append(MenuPROJECT_ADD_WIZARD, "Add new wizard");

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("wxDialog");
            ;
        },
        MenuPROJECT_ADD_DIALOG);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("wxDialog");
            wxGetFrame().CreateToolNode("wxNotebook");
            ;
        },
        MenuPROJECT_ADD_TABBED_DIALOG);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("wxFrame");
            ;
        },
        MenuPROJECT_ADD_WINDOW);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("wxWizard");
            ;
        },
        MenuPROJECT_ADD_WIZARD);
}

void NavPopupMenu::OnAddNew(wxCommandEvent& event)
{
    if (m_tool_name.size())
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
            m_child->CreateToolNode("wxBoxSizer");
            break;

        case MenuNEW_SIBLING_STATIC_SIZER:
            m_child->CreateToolNode("wxStaticBoxSizer");
            break;

        case MenuNEW_SIBLING_WRAP_SIZER:
            m_child->CreateToolNode("wxWrapSizer");
            break;

        case MenuNEW_SIBLING_GRID_SIZER:
            m_child->CreateToolNode("wxGridSizer");
            break;

        case MenuNEW_SIBLING_FLEX_GRID_SIZER:
            m_child->CreateToolNode("wxFlexGridSizer");
            break;

        case MenuNEW_SIBLING_GRIDBAG_SIZER:
            m_child->CreateToolNode("wxGridBagSizer");
            break;

        case MenuNEW_SIBLING_STD_DIALG_BTNS:
            m_child->CreateToolNode("wxStdDialogButtonSizer");
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
            wxGetFrame().CreateToolNode("wxBoxSizer");
            break;

        case MenuNEW_CHILD_STATIC_SIZER:
            wxGetFrame().CreateToolNode("wxStaticBoxSizer");
            break;

        case MenuNEW_CHILD_WRAP_SIZER:
            wxGetFrame().CreateToolNode("wxWrapSizer");
            break;

        case MenuNEW_CHILD_GRID_SIZER:
            wxGetFrame().CreateToolNode("wxGridSizer");
            break;

        case MenuNEW_CHILD_FLEX_GRID_SIZER:
            wxGetFrame().CreateToolNode("wxFlexGridSizer");
            break;

        case MenuNEW_CHILD_GRIDBAG_SIZER:
            wxGetFrame().CreateToolNode("wxGridBagSizer");
            break;

        case MenuNEW_CHILD_STD_DIALG_BTNS:
            wxGetFrame().CreateToolNode("wxStdDialogButtonSizer");
            break;

        case MenuNEW_CHILD_SPACER:
            wxGetFrame().CreateToolNode("spacer");
            break;

        case MenuNEW_TOOLBAR:
            wxGetFrame().CreateToolNode("wxToolBar");
            break;

        case MenuNEW_INFOBAR:
            wxGetFrame().CreateToolNode("wxInfoBar");
            break;

        case MenuADD_MENU:
            wxGetFrame().CreateToolNode("wxMenu");
            break;

        default:
            break;
    }
}

void NavPopupMenu::CreateContainerMenu(Node* node)
{
    Append(MenuCUT, "Cut\tCtrl+X");
    Append(MenuCOPY, "Copy\tCtrl+C");
    Append(MenuPASTE, "Paste\tCtrl+V");
    Append(MenuDELETE, "Delete\tCtrl+D");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    sub_menu->Append(MenuMOVE_UP, "Up\tAlt+Up", "Moves selected item up");
    sub_menu->Append(MenuMOVE_DOWN, "Down\tAlt+Down", "Moves selected item down");
    AppendSubMenu(sub_menu, "Move");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    if (node->GetClassName() == "BookPage")
    {
        if (!node->GetChildCount())
        {
            sub_menu = new wxMenu;
            sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
            sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
            sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
            sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
            sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
            sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
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
        sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
        sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
        sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
        sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
        sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
        sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");
        if (node->GetClassName() == "wxDialog")
        {
            sub_menu->AppendSeparator();
            sub_menu->Append(MenuNEW_SIBLING_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
        }
        AppendSubMenu(sub_menu, "&Add new sizer");

        if (node->GetClassName() == "wxWizardPageSimple")
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

    else if (node->GetClassName() == "wxWizardPageSimple")
    {
        sub_menu = new wxMenu;
        sub_menu->Append(MenuNEW_CHILD_BOX_SIZER, "wxBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_STATIC_SIZER, "wxStaticBoxSizer");
        sub_menu->Append(MenuNEW_CHILD_WRAP_SIZER, "wxWrapSizer");
        sub_menu->Append(MenuNEW_CHILD_GRID_SIZER, "wxGridSizer");
        sub_menu->Append(MenuNEW_CHILD_FLEX_GRID_SIZER, "wxFlexGridSizer");
        sub_menu->Append(MenuNEW_CHILD_GRIDBAG_SIZER, "wxGridBagSizer");
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

    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(m_node, MoveDirection::Up, true));
    Enable(MenuMOVE_DOWN, wxGetFrame().MoveNode(m_node, MoveDirection::Down, true));
}

void NavPopupMenu::CreateTopSizerMenu(Node* node)
{
    Append(MenuPASTE, "Paste\tCtrl+V");
    AppendSeparator();

    // Many of the OnAddNew commands add to a child, so we need to "fake" the child to ourselves
    m_child = node;

    auto sub_menu = new wxMenu;
    sub_menu->Append(MenuNEW_SIBLING_BOX_SIZER, "wxBoxSizer");
    sub_menu->Append(MenuNEW_SIBLING_STATIC_SIZER, "wxStaticBoxSizer");
    sub_menu->Append(MenuNEW_SIBLING_WRAP_SIZER, "wxWrapSizer");
    sub_menu->Append(MenuNEW_SIBLING_GRID_SIZER, "wxGridSizer");
    sub_menu->Append(MenuNEW_SIBLING_FLEX_GRID_SIZER, "wxFlexGridSizer");
    sub_menu->Append(MenuNEW_SIBLING_GRIDBAG_SIZER, "wxGridBagSizer");

    if (node->GetParent()->GetClassName() == "wxDialog")
    {
        sub_menu->AppendSeparator();
        sub_menu->Append(MenuNEW_SIBLING_STD_DIALG_BTNS, "wxStdDialogButtonSizer");
    }
    AppendSubMenu(sub_menu, "&Add new sizer");

    Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this);
    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, MenuPASTE);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);
}

void NavPopupMenu::CreateMenuMenu(Node* node)
{
    Append(MenuCUT, "Cut\tCtrl+X");
    Append(MenuCOPY, "Copy\tCtrl+C");
    Append(MenuPASTE, "Paste\tCtrl+V");
    Append(MenuDELETE, "Delete\tCtrl+D");

    AppendSeparator();
    Append(MenuMOVE_UP, "Move Up\tAlt+Up", "Moves selected item up");
    Append(MenuMOVE_DOWN, "Move Down\tAlt+Down", "Moves selected item down");

    AppendSeparator();
    Append(MenuADD_MENUITEM, "Add Menu &Item\tCtrl+I");
    Append(MenuADD_SUBMENU, "Add &Submenu &Item\tCtrl+S");
    Append(MenuADD_SEPARATOR, "Add S&eparator\tCtrl+E");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this);
    Bind(wxEVT_UPDATE_UI, &NavPopupMenu::OnUpdateEvent, this);

    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Up, true));
    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Down, true));

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("wxMenuItem");
            ;
            ;
        },
        MenuADD_MENUITEM);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("submenu");
            ;
        },
        MenuADD_SUBMENU);

    Bind(
        wxEVT_MENU,
        [](wxCommandEvent&) {
            wxGetFrame().CreateToolNode("separator");
            ;
        },
        MenuADD_SEPARATOR);
}

void NavPopupMenu::CreateBarMenu(Node* node)
{
    Append(MenuMOVE_UP, "Move Up\tAlt+Up", "Moves selected item up");
    Append(MenuMOVE_DOWN, "Move Down\tAlt+Down", "Moves selected item down");

    if (node->GetClassName() == "wxMenuBar" || node->GetClassName() == "MenuBar")
    {
        AppendSeparator();
        Append(MenuADD_MENU, "Add Menu\tCtrl+M");
    }
    else if (node->GetClassName() == "wxRibbonBar")
    {
        AppendSeparator();
        m_tool_name = "wxRibbonPage";
        Append(MenuNEW_ITEM, "Add Page");
    }
    else if (node->GetClassName() == "wxRibbonPage")
    {
        AppendSeparator();
        m_tool_name = "wxRibbonPanel";
        Append(MenuNEW_ITEM, "Add Panel");
    }
    else if (node->GetClassName() == "wxRibbonToolBar")
    {
        AppendSeparator();
        m_tool_name = "ribbonTool";
        Append(MenuNEW_ITEM, "Add Tool");
    }
    else if (node->GetClassName() == "ribbonTool")
    {
        m_child = node->GetParent();
        AppendSeparator();
        m_tool_name = "ribbonTool";
        Append(MenuNEW_ITEM, "Add Tool");
    }

    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Up, true));
    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Down, true));

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
    if (node->GetClassName() == "wxMenuBar" || node->GetClassName() == "MenuBar")
    {
        Bind(wxEVT_MENU, &NavPopupMenu::OnAddNew, this, MenuADD_MENU);
    }
}

void NavPopupMenu::CreateWizardMenu(Node* node)
{
    Append(MenuMOVE_UP, "Move Up\tAlt+Up", "Moves selected item up");
    Append(MenuMOVE_DOWN, "Move Down\tAlt+Down", "Moves selected item down");

    AppendSeparator();
    m_tool_name = "wxWizardPageSimple";
    Append(MenuNEW_ITEM, "Add Page\tCtrl+P");

    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Up, true));
    Enable(MenuMOVE_UP, wxGetFrame().MoveNode(node, MoveDirection::Down, true));

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
}

void NavPopupMenu::CreateBookMenu(Node* /* node */)
{
    m_tool_name = "BookPage";
    Append(MenuNEW_ITEM, "Add Page");
    AppendSeparator();

    auto sub_menu = new wxMenu;
    sub_menu->Append(MenuBORDERS_ALL, "All", "Borders on all sides", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_NONE, "None", "No borders", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_HORIZONTAL, "Horizontal only", "Borders only on left and right", wxITEM_CHECK);
    sub_menu->Append(MenuBORDERS_VERTICAL, "Vertical only", "Borders only on top and bottom", wxITEM_CHECK);
    AppendSubMenu(sub_menu, "Borders");

    Bind(wxEVT_MENU, &NavPopupMenu::OnMenuEvent, this, wxID_ANY);
}
