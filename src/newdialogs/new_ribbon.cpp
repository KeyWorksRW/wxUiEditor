/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newribbon_base.h"  // auto-generated: wxui/newribbon_base.h and wxui/newribbon_base.cpp

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MainFrame -- Main window frame
#include "new_common.h"              // Contains code common between all new_ dialogs
#include "node.h"                    // Node class
#include "node_creator.h"            // NodeCreator -- Class used to create nodes
#include "project_handler.h"         // ProjectHandler class
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewRibbon::OnInit(wxInitDialogEvent& event)
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

void NewRibbon::CreateNode()
{
    NodeSharedPtr bar_node;
    if (m_is_form)
    {
        bar_node = g_NodeCreator.CreateNode(gen_RibbonBar, Project.ProjectNode());
        ASSERT(bar_node);
    }
    else
    {
        bar_node = g_NodeCreator.CreateNode(gen_wxRibbonBar, wxGetFrame().GetSelectedNode());
        if (!bar_node)
        {
            wxMessageBox("You need to have a sizer selected before you can create a wxRibbonBar.", "Create wxRibbonBar");
            return;
        }
    }

    for (int count = 0; count < m_num_pages; ++count)
    {
        auto ribbon_page = g_NodeCreator.CreateNode(gen_wxRibbonPage, bar_node.get());
        bar_node->Adopt(ribbon_page);
        ttlib::cstr label("Page ");
        label << count + 1;
        ribbon_page->prop_set_value(prop_label, label);

        auto ribbon_panel = g_NodeCreator.CreateNode(gen_wxRibbonPanel, ribbon_page.get());
        ribbon_page->Adopt(ribbon_panel);
        label << ", panel 1";
        ribbon_panel->prop_set_value(prop_label, label);

        if (m_panel_type == "Tool")
        {
            auto tool_bar = g_NodeCreator.CreateNode(gen_wxRibbonToolBar, ribbon_panel.get());
            ribbon_panel->Adopt(tool_bar);
            auto tool = g_NodeCreator.CreateNode(gen_ribbonTool, tool_bar.get());
            tool_bar->Adopt(tool);
        }
        else if (m_panel_type == "Button")
        {
            auto button_bar = g_NodeCreator.CreateNode(gen_wxRibbonButtonBar, ribbon_panel.get());
            ribbon_panel->Adopt(button_bar);
            auto button = g_NodeCreator.CreateNode(gen_ribbonButton, button_bar.get());
            button_bar->Adopt(button);
        }
        else if (m_panel_type == "Gallery")
        {
            auto gallery_bar = g_NodeCreator.CreateNode(gen_wxRibbonGallery, ribbon_panel.get());
            ribbon_panel->Adopt(gallery_bar);
            auto item = g_NodeCreator.CreateNode(gen_ribbonGalleryItem, gallery_bar.get());
            gallery_bar->Adopt(item);
        }
    }

    if (!m_is_form)
    {
        auto parent = wxGetFrame().GetSelectedNode();
        auto pos = parent->FindInsertionPos(parent);
        ttlib::cstr undo_str("New wxRibbonBar");
        wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(bar_node.get(), parent, undo_str, pos));
    }
    else
    {
        bar_node->prop_set_value(prop_class_name, m_base_class.utf8_string());
        if (bar_node->prop_as_string(prop_class_name) != bar_node->prop_default_value(prop_class_name))
        {
            UpdateFormClass(bar_node.get());
        }

        wxGetFrame().SelectNode(Project.ProjectNode());

        ttlib::cstr undo_str("New wxRibbonBar");
        wxGetFrame().PushUndoAction(
            std::make_shared<InsertNodeAction>(bar_node.get(), Project.ProjectNode(), undo_str, -1));
    }

    wxGetFrame().FireCreatedEvent(bar_node);
    wxGetFrame().SelectNode(bar_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(bar_node.get(), true, true);

    // This probably already is activated, but let's be sure
    wxGetFrame().GetRibbonPanel()->ActivateBarPage();
}

bool NewRibbon::IsCreatable(bool notify_user)
{
    auto parent = wxGetFrame().GetSelectedNode();
    if (parent->IsSizer())
        return true;

    if (notify_user)
    {
        wxMessageBox("You need to have a sizer selected before you can create a wxRibbonBar.", "Create wxRibbonBar");
    }

    return false;
}

// Called whenever m_classname changes
void NewRibbon::VerifyClassName()
{
    if (!m_is_form)
        return;

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
