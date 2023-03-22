/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project wxFrame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newframe_base.h"  // auto-generated: wxui/newframe_base.h and wxui/newframe_base.cpp

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MoveDirection -- Main window frame
#include "new_common.h"              // Contains code common between all new_ dialogs
#include "node.h"                    // Node class
#include "node_creator.h"            // NodeCreator -- Class used to create nodes
#include "project_handler.h"         // ProjectHandler class
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewFrame::OnCheckMainFrame(wxCommandEvent& WXUNUSED(event))
{
    if (m_checkBox_mainframe->GetValue())
    {
        m_checkBox_toolbar->Enable();
        m_checkBox_menu->Enable();
        m_checkBox_statusbar->Enable();
    }
    else
    {
        m_checkBox_toolbar->Disable();
        m_checkBox_menu->Disable();
        m_checkBox_statusbar->Disable();
    }
}

void NewFrame::CreateNode()
{
    auto form_node = NodeCreation.CreateNode(gen_wxFrame, nullptr);
    ASSERT(form_node);

    if (m_has_mainframe)
    {
        if (m_has_toolbar)
        {
            auto bar = NodeCreation.CreateNode(gen_wxToolBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
        if (m_has_menu)
        {
            auto bar = NodeCreation.CreateNode(gen_wxMenuBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
        if (m_has_statusbar)
        {
            auto bar = NodeCreation.CreateNode(gen_wxStatusBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
    }

    form_node->prop_set_value(prop_class_name, m_base_class.utf8_string());
    if (form_node->prop_as_string(prop_class_name) != form_node->prop_default_value(prop_class_name))
    {
        UpdateFormClass(form_node.get());
    }

    auto parent_node = wxGetFrame().GetSelectedNode();
    if (!parent_node)
    {
        parent_node = Project.ProjectNode();
    }
    else
    {
        parent_node = parent_node->get_ValidFormParent();
    }

    wxGetFrame().SelectNode(parent_node);

    tt_string undo_str("New wxFrame");
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(form_node.get(), true, true);

    // If it's a mainframe then bars were probably added, so it makes sense to switch to the Bars ribbon bar page since
    // that's likely what the user will be doing next (adding tools or menus).
    if (m_has_mainframe)
        wxGetFrame().GetRibbonPanel()->ActivateBarPage();
}

// Called whenever m_classname changes
void NewFrame::VerifyClassName()
{
    if (!IsClassNameUnique(m_classname->GetValue()))
    {
        if (!m_is_info_shown)
        {
            m_infoBar->ShowMessage("This class name is already in use.", wxICON_WARNING);
            FindWindow(GetAffirmativeId())->Disable();
            Fit();
            m_is_info_shown = true;
        }
        return;
    }

    else if (m_is_info_shown)
    {
        m_is_info_shown = false;
        m_infoBar->Dismiss();
        FindWindow(GetAffirmativeId())->Enable();
        Fit();
    }
}
