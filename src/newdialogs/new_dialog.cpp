/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newdialog_base.h"  // auto-generated: wxui/newdialog_base.h and wxui/newdialog_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MainFrame -- Main window frame
#include "new_common.h"           // Contains code common between all new_ dialogs
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "project_handler.h"      // ProjectHandler class
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewDialog::OnInit(wxInitDialogEvent& event)
{
    // BUGBUG: [KeyWorks - 01-13-2022] Remove once issue #616 is fixed
    m_classname->SetFocus();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void NewDialog::CreateNode()
{
    auto form_node = g_NodeCreator.CreateNode(gen_wxDialog, nullptr);
    ASSERT(form_node);

    if (m_title.size())
    {
        form_node->prop_set_value(prop_title, m_title.utf8_string());
    }

    auto parent_sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, form_node.get());
    ASSERT(parent_sizer);
    parent_sizer->prop_set_value(prop_var_name, "dlg_sizer");
    form_node->Adopt(parent_sizer);

    if (m_has_tabs)
    {
        auto notebook = g_NodeCreator.CreateNode(gen_wxNotebook, parent_sizer.get());
        ASSERT(notebook);
        parent_sizer->Adopt(notebook);

        for (int count = 0; count < m_num_tabs; ++count)
        {
            auto book_page = g_NodeCreator.CreateNode(gen_BookPage, notebook.get());
            notebook->Adopt(book_page);

            ttlib::cstr label("Tab ");
            label << count + 1;
            book_page->prop_set_value(prop_label, label);
            auto page_sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, book_page.get());
            page_sizer->prop_set_value(prop_var_name, ttlib::cstr() << "page_sizer_" << count + 1);
            book_page->Adopt(page_sizer);
            auto static_text = g_NodeCreator.CreateNode(gen_wxStaticText, page_sizer.get());
            page_sizer->Adopt(static_text);
            static_text->prop_set_value(prop_label, "TODO: replace this control with something more useful...");
            static_text->prop_set_value(prop_wrap, "200");
        }
    }

    if (m_has_std_btns)
    {
        auto std_btn = g_NodeCreator.CreateNode(gen_wxStdDialogButtonSizer, parent_sizer.get());
        parent_sizer->Adopt(std_btn);

        std_btn->prop_set_value(prop_OK, "1");
        std_btn->prop_set_value(prop_Cancel, "1");
        std_btn->prop_set_value(prop_static_line, "1");
        std_btn->prop_set_value(prop_default_button, "OK");
        std_btn->prop_set_value(prop_flags, "wxEXPAND");
    }

    form_node->prop_set_value(prop_class_name, m_base_class.utf8_string());
    if (form_node->prop_as_string(prop_class_name) != form_node->prop_default_value(prop_class_name))
    {
        UpdateFormClass(form_node.get());
    }

    wxGetFrame().SelectNode(Project.ProjectNode());

    ttlib::cstr undo_str("New wxDialog");
    wxGetFrame().PushUndoAction(
        std::make_shared<InsertNodeAction>(form_node.get(), Project.ProjectNode(), undo_str, -1));
    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(form_node.get(), true, true);
}

// Called whenever m_classname changes
void NewDialog::VerifyClassName()
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
