/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new form panel
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newpanel_base.h"  // auto-generated: wxui/newpanel_base.h and wxui/newpanel_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MainFrame -- Main window frame
#include "new_common.h"           // Contains code common between all new_ dialogs
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "project_class.h"        // Project class
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewPanel::OnInit(wxInitDialogEvent& event)
{
    if (!m_is_form)
    {
        for (size_t idx = 0; idx < m_class_sizer->GetItemCount(); ++idx)
        {
            m_class_sizer->GetItem(idx)->GetWindow()->Hide();
        }
    }

    event.Skip();  // transfer all validator data to their windows and update UI
}

void NewPanel::CreateNode()
{
    NodeSharedPtr new_node;
    if (m_is_form)
    {
        new_node = g_NodeCreator.CreateNode(gen_PanelForm, GetProject());
        ASSERT(new_node);
    }
    else
    {
        new_node = g_NodeCreator.CreateNode(gen_wxPanel, wxGetFrame().GetSelectedNode());
        if (!new_node)
        {
            wxMessageBox("You need to have a sizer selected before you can create a wxPanel.", "Create wxPanel");
            return;
        }
    }

    NodeSharedPtr sizer = nullptr;

    if (m_sizer_type == "FlexGrid")
    {
        sizer = g_NodeCreator.CreateNode(gen_wxFlexGridSizer, new_node.get());
    }
    else if (m_sizer_type == "Grid")
    {
        sizer = g_NodeCreator.CreateNode(gen_wxGridSizer, new_node.get());
    }
    else if (m_sizer_type == "GridBag")
    {
        sizer = g_NodeCreator.CreateNode(gen_wxGridBagSizer, new_node.get());
    }
    else if (m_sizer_type == "StaticBox")
    {
        sizer = g_NodeCreator.CreateNode(gen_wxStaticBoxSizer, new_node.get());
    }
    else if (m_sizer_type == "Wrap")
    {
        sizer = g_NodeCreator.CreateNode(gen_wxWrapSizer, new_node.get());
    }
    else
    {
        sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, new_node.get());
    }

    new_node->Adopt(sizer);

    if (!m_tab_traversal)
    {
        new_node->prop_set_value(prop_window_style, "");
    }

    if (!m_is_form)
    {
        sizer->prop_set_value(prop_var_name, "panel_sizer");
        sizer->FixDuplicateName();

        auto parent = wxGetFrame().GetSelectedNode();
        auto pos = parent->FindInsertionPos(parent);
        ttlib::cstr undo_str("New wxPanel");
        wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent, undo_str, pos));
    }
    else
    {
        new_node->prop_set_value(prop_class_name, m_base_class.utf8_string());
        if (new_node->prop_as_string(prop_class_name) != new_node->prop_default_value(prop_class_name))
        {
            UpdateFormClass(new_node.get());
        }

        auto project = GetProject();
        wxGetFrame().SelectNode(project);

        ttlib::cstr undo_str("New wxPanel");
        wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), project, undo_str, -1));
    }

    wxGetFrame().FireCreatedEvent(new_node);
    wxGetFrame().SelectNode(new_node, evt_flags::fire_event & evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(new_node.get(), true, true);
}

// Called whenever m_classname changes
void NewPanel::VerifyClassName()
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
