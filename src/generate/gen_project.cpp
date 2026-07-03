/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include <utility>

#include "gen_base.h"         // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "node_prop.h"        // NodeProperty -- NodeProperty class
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction

#include "newdialogs/new_dialog.h"  // NewDialog -- Dialog for creating a new project dialog
#include "newdialogs/new_frame.h"   // NewFrame -- Dialog for creating a new project wxFrame
#include "newdialogs/new_wizard.h"  // NewWizard -- Dialog for creating a new wizard

#include "../panels/navpopupmenu.h"  // NavPopupMenu -- Context-menu for Navigation Panel

#include "gen_project.h"

bool ProjectGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddDialog), "Add new dialog...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddWindow), "Add new window...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddWizard), "Add new wizard...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddFolder), "Add folder");
    menu->AppendSeparator();
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectSortForms), "Sort Forms");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(Project.get_ProjectNode());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PushUndoAction(std::make_shared<SortProjectAction>());
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectSortForms));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddDialog));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_folder);
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddFolder));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWindow));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWizard));

    // Return false to indicate no sizer sub commands should be added
    return false;
}

bool ProjectGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop,
                                           Node* /* node */)
{
    if (prop->isProp(prop_cpp_line_length) || prop->isProp(prop_python_line_length) ||
        prop->isProp(prop_ruby_line_length) || prop->isProp(prop_fortran_line_length) ||
        prop->isProp(prop_go_line_length) || prop->isProp(prop_julia_line_length) ||
        prop->isProp(prop_lua_line_length) || prop->isProp(prop_typescript_line_length))
    {
        wxVariant const variant = event->GetPropertyValue();
        long const num = variant.GetInteger();
        if (num < 70)
        {
            event->SetValidationFailureMessage(
                "70 is the shortest line length that you can specify.");
            event->Veto();
            return false;
        }
    }
    return true;
}

bool FolderGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(std::to_underlying(NavPopupMenu::Menu::ProjectAddDialog), "Add new dialog...");
    menu->Append(std::to_underlying(NavPopupMenu::Menu::ProjectAddWindow), "Add new window...");
    menu->Append(std::to_underlying(NavPopupMenu::Menu::ProjectAddWizard), "Add new wizard...");
    menu->Append(std::to_underlying(NavPopupMenu::Menu::ProjectAddFolder), "Add folder");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(Project.get_ProjectNode());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddDialog));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_sub_folder);
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddFolder));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWindow));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWizard));

    return false;
}

bool SubFolderGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddDialog), "Add new dialog...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddWindow), "Add new window...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddWizard), "Add new wizard...");
    menu->Append(static_cast<int>(NavPopupMenu::Menu::ProjectAddFolder), "Add folder");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(Project.get_ProjectNode());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddDialog));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_sub_folder);
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddFolder));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWindow));

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().getWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        static_cast<int>(NavPopupMenu::Menu::ProjectAddWizard));

    return false;
}
