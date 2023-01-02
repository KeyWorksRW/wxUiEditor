/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"       // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "node_prop.h"      // NodeProperty -- NodeProperty class
#include "project_class.h"  // Project class
#include "undo_cmds.h"      // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "wxui/newdialog_base.h"  // NewDialog -- Dialog for creating a new project dialog
#include "wxui/newframe_base.h"   // NewFrame -- Dialog for creating a new project wxFrame
#include "wxui/newwizard_base.h"  // NewWizard -- Dialog for creating a new wizard

#include "../panels/navpopupmenu.h"  // NavPopupMenu -- Context-menu for Navigation Panel

#include "project.h"

bool ProjectGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_DIALOG, "Add new dialog...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WINDOW, "Add new window...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WIZARD, "Add new wizard...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_FOLDER, "Add folder");
    menu->AppendSeparator();
    menu->Append(NavPopupMenu::MenuPROJECT_SORT_FORMS, "Sort Forms");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(GetProject());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PushUndoAction(std::make_shared<SortProjectAction>());
        },
        NavPopupMenu::MenuPROJECT_SORT_FORMS);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_DIALOG);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_folder);
        },
        NavPopupMenu::MenuPROJECT_ADD_FOLDER);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WINDOW);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WIZARD);

    return true;
}

bool ProjectGenerator::AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* /* node */)
{
    if (prop->isProp(prop_cpp_line_length) || prop->isProp(prop_python_line_length))
    {
        auto variant = event->GetPropertyValue();
        auto num = variant.GetInteger();
        if (num < 70)
        {
            event->SetValidationFailureMessage("70 is the shortest line length that you can specify.");
            event->Veto();
            return false;
        }
    }
    return true;
}

bool FolderGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_DIALOG, "Add new dialog...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WINDOW, "Add new window...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WIZARD, "Add new wizard...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_FOLDER, "Add folder");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(GetProject());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_DIALOG);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_sub_folder);
        },
        NavPopupMenu::MenuPROJECT_ADD_FOLDER);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WINDOW);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WIZARD);

    return true;
}

bool SubFolderGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_DIALOG, "Add new dialog...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WINDOW, "Add new window...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WIZARD, "Add new wizard...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_FOLDER, "Add folder");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(GetProject());
        },
        wxID_PASTE);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewDialog dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_DIALOG);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().CreateToolNode(gen_sub_folder);
        },
        NavPopupMenu::MenuPROJECT_ADD_FOLDER);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewFrame dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WINDOW);

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            NewWizard dlg(wxGetFrame().GetWindow());
            if (dlg.ShowModal() == wxID_OK)
            {
                dlg.CreateNode();
            }
        },
        NavPopupMenu::MenuPROJECT_ADD_WIZARD);

    return true;
}
