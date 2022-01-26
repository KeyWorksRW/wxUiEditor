/////////////////////////////////////////////////////////////////////////////
// Purpose:   Project generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_base.h"   // BaseCodeGenerator -- Generate Src and Hdr files for Base Class
#include "mainframe.h"  // MainFrame -- Main window frame
#include "node.h"       // Node class
#include "node_prop.h"  // NodeProperty -- NodeProperty class
#include "undo_cmds.h"  // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "newdialog_base.h"  // NewDialog -- Dialog for creating a new project dialog
#include "newframe_base.h"   // NewFrame -- Dialog for creating a new project wxFrame
#include "newwizard_base.h"  // NewWizard -- Dialog for creating a new wizard

#include "../panels/navpopupmenu.h"  // NavPopupMenu -- Context-menu for Navigation Panel

#include "project.h"

bool ProjectGenerator::PopupMenuAddCommands(NavPopupMenu* menu, Node* /* node */)
{
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_DIALOG, "Add new dialog...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WINDOW, "Add new window...");
    menu->Append(NavPopupMenu::MenuPROJECT_ADD_WIZARD, "Add new wizard...");
    menu->AppendSeparator();
    menu->Append(NavPopupMenu::MenuPROJECT_SORT_FORMS, "Sort Forms");

    menu->Bind(
        wxEVT_MENU,
        [](wxCommandEvent&)
        {
            wxGetFrame().PasteNode(wxGetApp().GetProject());
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
