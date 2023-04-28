/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new MDI application
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newmdi_base.h"  // auto-generated: wxui/newframe_base.h and wxui/newframe_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MainFrame
#include "new_common.h"           // Contains code common between all new_ dialogs
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "project_handler.h"      // ProjectHandler class
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewMdiForm::OnViewType(wxCommandEvent& WXUNUSED(event)) {}

void NewMdiForm::OnOK(wxCommandEvent& WXUNUSED(event))
{
    if (!Validate() || !TransferDataFromWindow())
        return;

    if (m_view_type == "Text Control")
    {
        if (m_filter.empty())
        {
            m_filter = "*.txt";
        }
        if (m_default_extension.empty())
        {
            m_default_extension = "txt";
        }

        if (m_description.empty())
        {
            m_description = "Text";
        }

#if 0
        if (m_doc_name.empty())
        {
            m_doc_name = "Text Document";
        }
        if (m_view_name.empty())
        {
            m_view_name = "Text View";
        }
#endif
    }

    ASSERT(IsModal());
    EndModal(wxID_OK);
}

void NewMdiForm::CreateNode()
{
    auto folder = NodeCreation.CreateNode(gen_folder, nullptr);
    folder->prop_set_value(prop_label, get_folder_name());
    auto form_node = NodeCreation.CreateNode(gen_DocViewApp, folder.get());
    ASSERT(form_node);
    folder->Adopt(form_node);

    form_node->prop_set_value(prop_class_name, get_app_class().utf8_string());
    if (form_node->prop_as_string(prop_class_name) != form_node->prop_default_value(prop_class_name))
    {
        UpdateFormClass(form_node.get());
    }
    if (m_view_type == "Text Control")
    {
        auto doc_node = NodeCreation.CreateNode(gen_DocumentTextCtrl, form_node.get());
        ASSERT(doc_node);
        form_node->Adopt(doc_node);
        if (m_description.size())
        {
            doc_node->prop_set_value(prop_template_description, m_description);
        }
        if (m_filter.size())
        {
            doc_node->prop_set_value(prop_template_filter, m_filter);
        }
        if (m_default_extension.size())
        {
            doc_node->prop_set_value(prop_template_extension, m_default_extension);
        }
        if (get_view_class().size())
        {
            doc_node->prop_set_value(prop_template_view_name, get_view_class());
        }
        if (get_doc_class().size())
        {
            doc_node->prop_set_value(prop_template_doc_name, get_doc_class());
        }

        auto frame_menu = NodeCreation.CreateNode(gen_MdiFrameMenuBar, doc_node.get());
        ASSERT(frame_menu);

        auto file_menu = NodeCreation.CreateNode(gen_wxMenu, frame_menu.get());
        ASSERT(file_menu);
        file_menu->prop_set_value(prop_stock_id, "wxID_FILE");
        file_menu->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_FILE")));

        auto menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_NEW");
        menu_item->prop_set_value(prop_id, "wxID_NEW");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_NEW")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_NEW")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_OPEN");
        menu_item->prop_set_value(prop_id, "wxID_OPEN");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_OPEN")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_OPEN")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get());
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_EXIT");
        menu_item->prop_set_value(prop_id, "wxID_EXIT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_EXIT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_EXIT")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
        file_menu->Adopt(menu_item);

        auto help_menu = NodeCreation.CreateNode(gen_wxMenu, frame_menu.get());
        ASSERT(help_menu);
        help_menu->prop_set_value(prop_stock_id, "wxID_HELP");
        help_menu->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_HELP")));

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_ABOUT");
        menu_item->prop_set_value(prop_id, "wxID_ABOUT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_ABOUT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_ABOUT")));
        help_menu->Adopt(menu_item);

        frame_menu->Adopt(file_menu);
        frame_menu->Adopt(help_menu);

        doc_node->Adopt(frame_menu);
        auto doc_menu = NodeCreation.CreateNode(gen_MdiDocMenuBar, doc_node.get());
        ASSERT(doc_menu);
        file_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get());
        ASSERT(file_menu);
        file_menu->prop_set_value(prop_stock_id, "wxID_FILE");
        file_menu->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_FILE")));

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_NEW");
        menu_item->prop_set_value(prop_id, "wxID_NEW");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_NEW")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_NEW")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_OPEN");
        menu_item->prop_set_value(prop_id, "wxID_OPEN");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_OPEN")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_OPEN")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_SAVE");
        menu_item->prop_set_value(prop_id, "wxID_SAVE");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_SAVE")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_SAVE")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_FILE_SAVE|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_SAVEAS");
        menu_item->prop_set_value(prop_id, "wxID_SAVEAS");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_SAVEAS")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_SAVEAS")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_FILE_SAVE_AS|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get());
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_PRINT");
        menu_item->prop_set_value(prop_id, "wxID_PRINT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_PRINT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_PRINT")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_PRINT|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get());
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_CLOSE");
        menu_item->prop_set_value(prop_id, "wxID_CLOSE");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_CLOSE")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_CLOSE")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_CLOSE|wxART_MENU");
        file_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_EXIT");
        menu_item->prop_set_value(prop_id, "wxID_EXIT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_EXIT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_EXIT")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
        file_menu->Adopt(menu_item);

        auto edit_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get());
        ASSERT(edit_menu);
        edit_menu->prop_set_value(prop_stock_id, "wxID_EDIT");
        edit_menu->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_EDIT")));

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_CUT");
        menu_item->prop_set_value(prop_id, "wxID_CUT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_CUT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_CUT")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_CUT|wxART_MENU");
        edit_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_COPY");
        menu_item->prop_set_value(prop_id, "wxID_COPY");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_COPY")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_COPY")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_COPY|wxART_MENU");
        edit_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_PASTE");
        menu_item->prop_set_value(prop_id, "wxID_PASTE");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_PASTE")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_PASTE")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxID_PASTE|wxART_MENU");
        edit_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get());
        edit_menu->Adopt(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_SELECTALL");
        menu_item->prop_set_value(prop_id, "wxID_SELECTALL");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_SELECTALL")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_SELECTALL")));
        menu_item->prop_set_value(prop_bitmap, "Art;wxART_PASTE|wxART_MENU");
        edit_menu->Adopt(menu_item);

        help_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get());
        ASSERT(help_menu);
        help_menu->prop_set_value(prop_label, "wxID_HELP");
        help_menu->prop_set_value(prop_stock_id, "wxID_HELP");

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get());
        menu_item->prop_set_value(prop_stock_id, "wxID_ABOUT");
        menu_item->prop_set_value(prop_id, "wxID_ABOUT");
        menu_item->prop_set_value(prop_label, wxGetStockLabel(NodeCreation.GetConstantAsInt("wxID_ABOUT")));
        menu_item->prop_set_value(prop_help, wxGetStockHelpString(NodeCreation.GetConstantAsInt("wxID_ABOUT")));
        help_menu->Adopt(menu_item);

        doc_menu->Adopt(file_menu);
        doc_menu->Adopt(edit_menu);
        doc_menu->Adopt(help_menu);

        doc_node->Adopt(doc_menu);
        auto view = NodeCreation.CreateNode(gen_ViewTextCtrl, doc_node.get());
        ASSERT(view);
        doc_node->Adopt(view);
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

    tt_string undo_str("New MDI App");

    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(folder.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(folder);
    wxGetFrame().SelectNode(folder, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(folder.get(), true, true);
}

// Called whenever m_classname changes
void NewMdiForm::VerifyClassName() {}
