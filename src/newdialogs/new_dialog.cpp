/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new project dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "../ui/newdialog_base.h"  // auto-generated: ../ui/newdialog_base.h and ../ui/newdialog_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MainFrame -- Main window frame
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_textCtrl_title->SetFocus();
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
            if (wxGetApp().GetProject()->HasValue(prop_base_directory))
                base_file.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/');
            form_node->prop_set_value(prop_base_file, base_file);

            base_file.Replace("_base", "");
            if (wxGetApp().GetProject()->HasValue(prop_base_directory))
            {
                base_file.erase(0, (wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/').size());
                base_file.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_derived_directory) << '/');
            }
            form_node->prop_set_value(prop_derived_file, base_file);
        }
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

    ttlib::cstr undo_str("New Dialog");

    auto parent = wxGetApp().GetProject();
    wxGetFrame().SelectNode(parent);

    auto pos = parent->FindInsertionPos(parent);
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent, undo_str, pos));

    if (form_node->prop_as_string(prop_class_name) != form_node->prop_default_value(prop_class_name))
    {
        bool is_base_class = false;
        ttString baseName = form_node->prop_as_wxString(prop_class_name);
        if (baseName.Right(4) == "Base")
        {
            baseName.Replace("Base", wxEmptyString);
            is_base_class = true;
        }
        baseName.MakeLower();
        baseName << "_base";
        if (wxGetApp().GetProject()->HasValue(prop_base_directory))
            baseName.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/');

        form_node->prop_set_value(prop_base_file, baseName);
        if (is_base_class)
        {
            form_node->prop_set_value(prop_base_file, baseName);

            wxString class_name = form_node->prop_as_wxString(prop_class_name);
            if (class_name.Right(4) == "Base")
            {
                class_name.Replace("Base", wxEmptyString);
            }
            else
            {
                class_name << "Derived";
            }
            form_node->prop_set_value(prop_derived_class_name, class_name);

            ttString drvName = form_node->prop_as_wxString(prop_derived_class_name);
            if (drvName.Right(7) == "Derived")
                drvName.Replace("Derived", "_derived");
            else if (!is_base_class)
            {
                drvName << "_derived";
            }

            drvName.MakeLower();
            if (wxGetApp().GetProject()->HasValue(prop_base_directory))
                drvName.insert(0, wxGetApp().GetProject()->prop_as_wxString(prop_base_directory) << '/');

            form_node->prop_set_value(prop_derived_file, drvName);
        }
    }

    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, true, true);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(form_node.get(), true, true);
}
