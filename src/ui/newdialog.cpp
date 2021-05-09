/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include "newdialog.h"  // auto-generated: newdialog_base.h and newdialog_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MoveDirection -- Main window frame
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

NewDialog::NewDialog(wxWindow* parent) : NewDialogBase(parent) {}

void NewDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_textCtrl_title->SetFocus();
}

inline void Adopt(NodeSharedPtr parent, NodeSharedPtr child)
{
    parent->AddChild(child);
    child->SetParent(parent);
}

void NewDialog::CreateNode()
{
    auto form_node = g_NodeCreator.CreateNode(gen_wxDialog, nullptr);
    ASSERT(form_node);

    if (m_title.size())
    {
        form_node->prop_set_value(prop_title, m_title.utf8_string());
    }

    if (m_base_class != "MyDialogBase")
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

    auto parent_sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, form_node.get());
    ASSERT(parent_sizer);
    Adopt(form_node, parent_sizer);

    if (m_has_tabs)
    {
        auto notebook = g_NodeCreator.CreateNode(gen_wxNotebook, parent_sizer.get());
        ASSERT(notebook);
        Adopt(parent_sizer, notebook);

        for (int count = 0; count < m_num_tabs; ++count)
        {
            auto book_page = g_NodeCreator.CreateNode(gen_BookPage, notebook.get());
            Adopt(notebook, book_page);

            ttlib::cstr label("Tab ");
            label << count + 1;
            book_page->prop_set_value(prop_label, label);
            auto page_sizer = g_NodeCreator.CreateNode(gen_VerticalBoxSizer, book_page.get());
            Adopt(book_page, page_sizer);
            auto static_text = g_NodeCreator.CreateNode(gen_wxStaticText, page_sizer.get());
            Adopt(page_sizer, static_text);
            static_text->prop_set_value(prop_label, "TODO: replace this control with something more useful...");
            static_text->prop_set_value(prop_wrap, "200");
        }
    }

    if (m_has_std_btns)
    {
        auto std_btn = g_NodeCreator.CreateNode(gen_wxStdDialogButtonSizer, parent_sizer.get());
        Adopt(parent_sizer, std_btn);

        std_btn->prop_set_value(prop_OK, "1");
        std_btn->prop_set_value(prop_Cancel, "1");
        std_btn->prop_set_value(prop_static_line, "1");
        std_btn->prop_set_value(prop_default_button, "OK");
        std_btn->prop_set_value(prop_flags, "wxEXPAND");
    }

    ttlib::cstr undo_str("New Dialog");

    auto parent = wxGetApp().GetProject();
    wxGetFrame().SelectNode(parent);

    auto pos = parent->FindInsertionPos(parent);
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent, undo_str, pos));
    form_node->FixPastedNames();

    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, true, true);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(form_node.get(), true, true);
}
