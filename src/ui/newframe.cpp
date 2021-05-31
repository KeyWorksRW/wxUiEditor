/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project wxFrame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MoveDirection -- Main window frame
#include "node.h"                    // Node class
#include "node_creator.h"            // NodeCreator -- Class used to create nodes
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "newframe.h"  // auto-generated: newframe_base.h and newframe_base.cpp

NewFrame::NewFrame(wxWindow* parent) : NewFrameBase(parent) {}

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
    auto form_node = g_NodeCreator.CreateNode(gen_wxFrame, nullptr);
    ASSERT(form_node);

    if (m_base_class != "MyFrameBase")
    {
        form_node->prop_set_value(prop_class_name, m_base_class.utf8_string());
        if (m_base_class.Right(4) == "Base")
        {
            wxString derived_class = m_base_class;
            derived_class.Replace("Base", wxEmptyString);
            form_node->prop_set_value(prop_derived_class_name, derived_class);

            ttString base_file = derived_class;
            base_file.MakeLower();
            base_file << "_base";
            form_node->prop_set_value(prop_base_file, base_file);

            base_file.Replace("_base", "");
            form_node->prop_set_value(prop_derived_file, base_file);
        }
    }

    if (m_has_mainframe)
    {
        if (m_has_toolbar)
        {
            auto bar = g_NodeCreator.CreateNode(gen_wxToolBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
        if (m_has_menu)
        {
            auto bar = g_NodeCreator.CreateNode(gen_wxMenuBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
        if (m_has_statusbar)
        {
            auto bar = g_NodeCreator.CreateNode(gen_wxStatusBar, form_node.get());
            ASSERT(bar);
            form_node->Adopt(bar);
        }
    }

    ttlib::cstr undo_str("New wxFrame");

    auto parent = wxGetApp().GetProject();
    wxGetFrame().SelectNode(parent);

    auto pos = parent->FindInsertionPos(parent);
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent, undo_str, pos));
    form_node->FixDuplicateNodeNames();

    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, true, true);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(form_node.get(), true, true);

    // If it's a mainframe then bars were probably added, so it makes sense to switch to the Bars ribbon bar page since
    // that's likely what the user will be doing next (adding tools or menus).
    if (m_has_mainframe)
        wxGetFrame().GetRibbonPanel()->ActivateBarPage();
}
