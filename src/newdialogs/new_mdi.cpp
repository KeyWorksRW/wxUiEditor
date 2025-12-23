/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new MDI application
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "new_mdi.h"

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "gen_enums.h"            // Enumerations for generators
#include "mainframe.h"            // MainFrame
#include "new_common.h"           // Contains code common between all new_ dialogs
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "project_handler.h"      // ProjectHandler class
#include "undo_cmds.h"            // Undoable command classes derived from UndoAction

auto NewMdiForm::OnOK(wxCommandEvent& /* event unused */) -> void
{
    if (!Validate() || !TransferDataFromWindow())
        return;

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

    ASSERT(IsModal());
    EndModal(wxID_OK);
}

namespace
{
    enum
    {
        DOC_TYPE_IMAGE,
        DOC_TYPE_RICHTEXT,
        DOC_TYPE_STYLED,
        DOC_TYPE_TEXT,
        DOC_TYPE_SPLITTER,
        DOC_TYPE_UNKNOWN
    };
}

auto NewMdiForm::CreateNode() -> void
{
    auto folder = NodeCreation.CreateNode(gen_folder, nullptr).first;
    folder->set_value(prop_label, get_folder_name());
    auto app_node = NodeCreation.CreateNode(gen_DocViewApp, folder.get()).first;
    ASSERT(app_node);
    folder->AdoptChild(app_node);

    app_node->set_value(prop_class_name, get_app_class().utf8_string());
    if (app_node->as_string(prop_class_name) != app_node->get_PropDefaultValue(prop_class_name))
    {
        UpdateFormClass(app_node.get());
    }

    auto doc_type = DOC_TYPE_UNKNOWN;
    if (m_view_type == "Image")
        doc_type = DOC_TYPE_IMAGE;
    else if (m_view_type == "wxRichTextCtrl")
        doc_type = DOC_TYPE_RICHTEXT;
    else if (m_view_type == "wxStyledTextCtrl")
        doc_type = DOC_TYPE_STYLED;
    else if (m_view_type == "wxTextCtrl")
        doc_type = DOC_TYPE_TEXT;
    else if (m_view_type == "wxSplitterWindow")
        doc_type = DOC_TYPE_SPLITTER;

    if (doc_type == DOC_TYPE_UNKNOWN)
    {
        wxMessageBox("Unrecognized document type", "Error", wxOK | wxICON_ERROR);
        return;
    }

    auto frame_menu = NodeCreation.CreateNode(gen_MdiFrameMenuBar, app_node.get()).first;
    ASSERT(frame_menu);

    auto file_menu = NodeCreation.CreateNode(gen_wxMenu, frame_menu.get()).first;
    ASSERT(file_menu);
    file_menu->set_value(prop_stock_id, "wxID_FILE");
    file_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_FILE")));

    auto menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_NEW");
    menu_item->set_value(prop_id, "wxID_NEW");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_NEW")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_NEW")));
    menu_item->set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_OPEN");
    menu_item->set_value(prop_id, "wxID_OPEN");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_OPEN")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_OPEN")));
    menu_item->set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get()).first;
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_EXIT");
    menu_item->set_value(prop_id, "wxID_EXIT");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_EXIT")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_EXIT")));
    menu_item->set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    auto help_menu = NodeCreation.CreateNode(gen_wxMenu, frame_menu.get()).first;
    ASSERT(help_menu);
    help_menu->set_value(prop_stock_id, "wxID_HELP");
    help_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_HELP")));

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_ABOUT");
    menu_item->set_value(prop_id, "wxID_ABOUT");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_ABOUT")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_ABOUT")));
    help_menu->AdoptChild(menu_item);

    frame_menu->AdoptChild(file_menu);
    frame_menu->AdoptChild(help_menu);

    app_node->AdoptChild(frame_menu);

    // Now create the document node and it's menu bar and view class

    auto doc_node = NodeCreation.CreateNode(gen_DocumentTextCtrl, app_node.get()).first;
    ASSERT(doc_node);
    switch (doc_type)
    {
        case DOC_TYPE_IMAGE:
            doc_node->set_value(GenEnum::prop_base_file, "docview_doc_image.cpp");
            break;
        case DOC_TYPE_RICHTEXT:
            doc_node->set_value(GenEnum::prop_base_file, "docview_doc_richtext.cpp");
            break;
        case DOC_TYPE_STYLED:
            doc_node->set_value(GenEnum::prop_base_file, "docview_doc_styled.cpp");
            break;
        case DOC_TYPE_TEXT:
            doc_node->set_value(GenEnum::prop_base_file, "docview_doc_textctrl.cpp");
            break;
        case DOC_TYPE_SPLITTER:
            doc_node->set_value(GenEnum::prop_base_file, "docview_doc_splitter.cpp");
            break;
        default:
            break;
    }
    app_node->AdoptChild(doc_node);
    doc_node->set_value(prop_mdi_class_name, app_node->as_string(prop_class_name));
    if (m_description.size())
    {
        doc_node->set_value(prop_template_description, m_description);
    }
    if (m_filter.size())
    {
        doc_node->set_value(prop_template_filter, m_filter);
    }
    if (m_default_extension.size())
    {
        doc_node->set_value(prop_template_extension, m_default_extension);
    }
    if (get_view_class().size())
    {
        doc_node->set_value(prop_template_view_name, get_view_class());
    }
    if (get_doc_class().size())
    {
        doc_node->set_value(prop_template_doc_name, get_doc_class());
    }
    auto doc_menu = NodeCreation.CreateNode(gen_MdiDocMenuBar, doc_node.get()).first;
    ASSERT(doc_menu);
    file_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get()).first;
    ASSERT(file_menu);
    file_menu->set_value(prop_stock_id, "wxID_FILE");
    file_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_FILE")));

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_NEW");
    menu_item->set_value(prop_id, "wxID_NEW");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_NEW")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_NEW")));
    menu_item->set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_OPEN");
    menu_item->set_value(prop_id, "wxID_OPEN");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_OPEN")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_OPEN")));
    menu_item->set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_SAVE");
    menu_item->set_value(prop_id, "wxID_SAVE");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_SAVE")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_SAVE")));
    menu_item->set_value(prop_bitmap, "Art;wxART_FILE_SAVE|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_SAVEAS");
    menu_item->set_value(prop_id, "wxID_SAVEAS");
    menu_item->set_value(prop_label,
                         wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_SAVEAS")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_SAVEAS")));
    menu_item->set_value(prop_bitmap, "Art;wxART_FILE_SAVE_AS|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get()).first;
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_PRINT");
    menu_item->set_value(prop_id, "wxID_PRINT");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_PRINT")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_PRINT")));
    menu_item->set_value(prop_bitmap, "Art;wxART_PRINT|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get()).first;
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_CLOSE");
    menu_item->set_value(prop_id, "wxID_CLOSE");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_CLOSE")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_CLOSE")));
    menu_item->set_value(prop_bitmap, "Art;wxART_CLOSE|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_EXIT");
    menu_item->set_value(prop_id, "wxID_EXIT");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_EXIT")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_EXIT")));
    menu_item->set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
    file_menu->AdoptChild(menu_item);

    NodeSharedPtr edit_menu;
    if (doc_type != DOC_TYPE_IMAGE)
    {
        edit_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get()).first;
        ASSERT(edit_menu);
        edit_menu->set_value(prop_stock_id, "wxID_EDIT");
        edit_menu->set_value(prop_label,
                             wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_EDIT")));

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
        menu_item->set_value(prop_stock_id, "wxID_CUT");
        menu_item->set_value(prop_id, "wxID_CUT");
        menu_item->set_value(prop_label,
                             wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_CUT")));
        menu_item->set_value(prop_help,
                             wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_CUT")));
        menu_item->set_value(prop_bitmap, "Art;wxART_CUT|wxART_MENU");
        edit_menu->AdoptChild(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
        menu_item->set_value(prop_stock_id, "wxID_COPY");
        menu_item->set_value(prop_id, "wxID_COPY");
        menu_item->set_value(prop_label,
                             wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_COPY")));
        menu_item->set_value(prop_help,
                             wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_COPY")));
        menu_item->set_value(prop_bitmap, "Art;wxART_COPY|wxART_MENU");
        edit_menu->AdoptChild(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
        menu_item->set_value(prop_stock_id, "wxID_PASTE");
        menu_item->set_value(prop_id, "wxID_PASTE");
        menu_item->set_value(prop_label,
                             wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_PASTE")));
        menu_item->set_value(prop_help,
                             wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_PASTE")));
        menu_item->set_value(prop_bitmap, "Art;wxID_PASTE|wxART_MENU");
        edit_menu->AdoptChild(menu_item);

        menu_item = NodeCreation.CreateNode(gen_separator, file_menu.get()).first;
        edit_menu->AdoptChild(menu_item);

        menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
        menu_item->set_value(prop_stock_id, "wxID_SELECTALL");
        menu_item->set_value(prop_id, "wxID_SELECTALL");
        menu_item->set_value(prop_label,
                             wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_SELECTALL")));
        menu_item->set_value(
            prop_help, wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_SELECTALL")));
        menu_item->set_value(prop_bitmap, "Art;wxART_PASTE|wxART_MENU");
        edit_menu->AdoptChild(menu_item);
    }

    help_menu = NodeCreation.CreateNode(gen_wxMenu, doc_menu.get()).first;
    ASSERT(help_menu);
    help_menu->set_value(prop_label, "wxID_HELP");
    help_menu->set_value(prop_stock_id, "wxID_HELP");

    menu_item = NodeCreation.CreateNode(gen_wxMenuItem, file_menu.get()).first;
    menu_item->set_value(prop_stock_id, "wxID_ABOUT");
    menu_item->set_value(prop_id, "wxID_ABOUT");
    menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.get_ConstantAsInt("wxID_ABOUT")));
    menu_item->set_value(prop_help,
                         wxGetStockHelpString(NodeCreation.get_ConstantAsInt("wxID_ABOUT")));
    help_menu->AdoptChild(menu_item);

    doc_menu->AdoptChild(file_menu);
    if (edit_menu)
    {
        doc_menu->AdoptChild(edit_menu);
    }
    doc_menu->AdoptChild(help_menu);

    doc_node->AdoptChild(doc_menu);

    // Now create the view node

    GenName gen_view_name;
    switch (doc_type)
    {
        case DOC_TYPE_IMAGE:
            gen_view_name = gen_ViewImage;
            break;
        case DOC_TYPE_RICHTEXT:
            gen_view_name = gen_ViewRichTextCtrl;
            break;
        case DOC_TYPE_STYLED:
            gen_view_name = gen_ViewStyledTextCtrl;
            break;
        case DOC_TYPE_TEXT:
            gen_view_name = gen_ViewTextCtrl;
            break;
        case DOC_TYPE_SPLITTER:
            gen_view_name = gen_ViewSplitterWindow;
            break;
        default:
            gen_view_name = gen_unknown;
            break;
    }

    auto view = NodeCreation.CreateNode(gen_view_name, doc_node.get()).first;
    ASSERT(view);
    view->set_value(prop_class_name, get_view_class());
    doc_node->AdoptChild(view);

    auto parent_node = wxGetFrame().getSelectedNode();
    if (!parent_node)
    {
        parent_node = Project.get_ProjectNode();
    }
    else
    {
        parent_node = parent_node->get_ValidFormParent();
    }

    wxGetFrame().SelectNode(parent_node);

    tt_string undo_str("New MDI Form");

    wxGetFrame().PushUndoAction(
        std::make_shared<InsertNodeAction>(folder.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(folder);
    wxGetFrame().SelectNode(folder, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().getNavigationPanel()->ChangeExpansion(folder.get(), true, true);
}

// Called whenever m_classname changes
auto NewMdiForm::VerifyClassName() -> void
{
    // Check all class names to ensure they are unique
    bool has_duplicate = false;
    wxString duplicate_name;

    // Check app class name
    if (!IsClassNameUnique(m_app_class))
    {
        has_duplicate = true;
        duplicate_name = m_app_class;
    }
    // Check doc class name
    else if (!IsClassNameUnique(m_doc_class))
    {
        has_duplicate = true;
        duplicate_name = m_doc_class;
    }
    // Check view class name
    else if (!IsClassNameUnique(m_view_class))
    {
        has_duplicate = true;
        duplicate_name = m_view_class;
    }

    if (has_duplicate)
    {
        wxMessageBox(wxString::Format("The class name \"%s\" is already in use by another form.",
                                      duplicate_name),
                     "Duplicate Class Name", wxOK | wxICON_ERROR, this);
        return;
    }
}

auto NewMdiForm::OnViewType(wxCommandEvent& /* event unused */) -> void
{
    auto view_type = m_choice_view_type->GetStringSelection();
    if (view_type == "Image")
    {
        m_txtctrl_view_classname->SetValue("ImageViewBase");
        m_description = "Image";
        m_filter = "*.png;*.jpg;*.jpeg";
        m_default_extension = "png";
        m_doc_class = m_app_class + "Doc";
        m_view_class = m_app_class + "View";
    }
    else if (view_type == "wxRichTextCtrl")
    {
        m_txtctrl_view_classname->SetValue("RichTextViewBase");
        m_description = "Rich Text";
        m_filter = "*.rtf";
        m_default_extension = "rtf";
        m_doc_class = m_app_class + "Doc";
        m_view_class = m_app_class + "View";
    }
    else if (view_type == "wxStyledTextCtrl")
    {
        m_txtctrl_view_classname->SetValue("StyledViewBase");
        m_description = "Text";
        m_filter = "*.txt";
        m_default_extension = "txt";
        m_doc_class = m_app_class + "Doc";
        m_view_class = m_app_class + "View";
    }
    else if (view_type == "wxTextCtrl")
    {
        m_txtctrl_view_classname->SetValue("TextViewBase");
        m_description = "Text";
        m_filter = "*.txt";
        m_default_extension = "txt";
        m_doc_class = m_app_class + "Doc";
        m_view_class = m_app_class + "View";
    }
    else if (view_type == "wxSplitterWindow")
    {
        m_txtctrl_view_classname->SetValue("SplitterViewBase");
        m_description = "Splitter";
        m_filter = "*.txt";
        m_default_extension = "txt";
        m_doc_class = m_app_class + "Doc";
        m_view_class = m_app_class + "View";
    }
}
