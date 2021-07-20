/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new wxRibbonBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "newribbon.h"  // auto-generated: newribbon_base.h and newribbon_base.cpp

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MainFrame -- Main window frame
#include "node.h"                    // Node class
#include "node_creator.h"            // NodeCreator -- Class used to create nodes
#include "uifuncs.h"                 // Miscellaneous functions for displaying UI
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

NewRibbon::NewRibbon(wxWindow* parent) : NewRibbonBase(parent)
{
    // TODO: [KeyWorks - 05-09-2021] Remove once issue #212 is fixed.
    m_panel_type = "Tool";
}

void NewRibbon::CreateNode()
{
    NodeSharedPtr bar_node;
    if (m_is_form)
    {
        bar_node = g_NodeCreator.CreateNode(gen_RibbonBar, wxGetApp().GetProject());
        ASSERT(bar_node);
    }
    else
    {
        bar_node = g_NodeCreator.CreateNode(gen_wxRibbonBar, wxGetFrame().GetSelectedNode());
        if (!bar_node)
        {
            appMsgBox("You need to have a sizer selected before you can create a wxRibbonBar.", "Create wxRibbonBar");
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

    ttlib::cstr undo_str("New wxRibbonBar");

    auto parent = wxGetFrame().GetSelectedNode();
    auto pos = parent->FindInsertionPos(parent);
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(bar_node.get(), parent, undo_str, pos));
    wxGetFrame().FireCreatedEvent(bar_node);
    wxGetFrame().SelectNode(bar_node, true, true);
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
        appMsgBox("You need to have a sizer selected before you can create a wxRibbonBar.", "Create wxRibbonBar");
    }

    return false;
}
