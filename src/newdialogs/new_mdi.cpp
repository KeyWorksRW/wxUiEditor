///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/choice.h>
#include <wx/radiobut.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "new_mdi.h"

bool NewMdiForm::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* box_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto* staticText_3 = new wxStaticText(this, wxID_ANY,
        "These are initial values -- all of them can be changed after the frame is created.");
    staticText_3->Wrap(300);
    box_sizer_3->Add(staticText_3, wxSizerFlags().Border(wxALL));

    m_infoBar = new wxInfoBar(this);
    m_infoBar->SetShowHideEffects(wxSHOW_EFFECT_NONE, wxSHOW_EFFECT_NONE);
    m_infoBar->SetEffectDuration(500);
    box_sizer_3->Add(m_infoBar, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer_7 = new wxBoxSizer(wxHORIZONTAL);

    auto* box_sizer_8 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_6 = new wxStaticText(this, wxID_ANY, "&Folder name:");
    staticText_6->SetToolTip("Change this to something unique to your project.");
    box_sizer_8->Add(staticText_6, wxSizerFlags().Center().Border(wxALL));

    auto* folder_name = new wxTextCtrl(this, wxID_ANY, "Mdi Application");
    folder_name->SetFocus();
    folder_name->SetValidator(wxTextValidator(wxFILTER_NONE, &m_folder_name));
    folder_name->SetToolTip("Change this to something unique to your project.");
    box_sizer_8->Add(folder_name, wxSizerFlags(1).Border(wxALL));

    box_sizer_7->Add(box_sizer_8, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer_2 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_9 = new wxStaticText(this, wxID_ANY, "&App class:");
    staticText_9->SetToolTip("Change this to something unique to your project.");
    box_sizer_2->Add(staticText_9, wxSizerFlags().Center().Border(wxALL));

    auto* app_classname = new wxTextCtrl(this, wxID_ANY, "DocViewAppBase");
    app_classname->SetValidator(wxTextValidator(wxFILTER_NONE, &m_app_class));
    app_classname->SetMinSize(ConvertDialogToPixels(wxSize(100, -1)));
    app_classname->SetToolTip("Change this to something unique to your project.");
    box_sizer_2->Add(app_classname, wxSizerFlags(1).Border(wxALL));

    box_sizer_7->Add(box_sizer_2, wxSizerFlags().Expand().Border(wxALL));

    box_sizer_3->Add(box_sizer_7, wxSizerFlags().Border(wxALL));

    auto* static_box = new wxStaticBoxSizer(wxHORIZONTAL, this, "Frame Type");

    auto* radioBtn = new wxRadioButton(static_box->GetStaticBox(), wxID_ANY, "wxAuiMDIParentFrame", wxDefaultPosition,
        wxDefaultSize, wxRB_GROUP);
    radioBtn->SetValue(true);
    radioBtn->SetValidator(wxGenericValidator(&m_aui_frame));
    static_box->Add(radioBtn, wxSizerFlags().Border(wxALL));

    auto* radioBtn_2 = new wxRadioButton(static_box->GetStaticBox(), wxID_ANY, "wxDocMDIParentFrame");
    radioBtn_2->SetValidator(wxGenericValidator(&m_doc_frame));
    static_box->Add(radioBtn_2, wxSizerFlags().Border(wxALL));

    box_sizer_3->Add(static_box, wxSizerFlags().Expand().Border(wxALL));

    auto* static_box_2 = new wxStaticBoxSizer(wxHORIZONTAL, this, "View");

    auto* choice_view = new wxChoice(static_box_2->GetStaticBox(), wxID_ANY);
    choice_view->Append("Text Control");
    choice_view->Append("Image");
    m_view_type = "Text Control";  // set validator variable
    choice_view->SetValidator(wxGenericValidator(&m_view_type));
    static_box_2->Add(choice_view, wxSizerFlags(1).Border(wxALL));

    auto* staticText_11 = new wxStaticText(static_box_2->GetStaticBox(), wxID_ANY, "Class name:");
    staticText_11->SetToolTip("Change this to something unique to your project.");
    static_box_2->Add(staticText_11, wxSizerFlags().Center().Border(wxALL));

    auto* view_classname = new wxTextCtrl(static_box_2->GetStaticBox(), wxID_ANY, "TextViewBase");
    view_classname->SetValidator(wxTextValidator(wxFILTER_NONE, &m_view_class));
    view_classname->SetToolTip("Change this to something unique to your project.");
    static_box_2->Add(view_classname, wxSizerFlags(1).Border(wxALL));

    box_sizer_3->Add(static_box_2, wxSizerFlags().Expand().Border(wxALL));

    auto* static_box_3 = new wxStaticBoxSizer(wxVERTICAL, this, "Document");

    auto* flex_grid_sizer = new wxFlexGridSizer(4, 0, 0);
    flex_grid_sizer->SetFlexibleDirection(wxHORIZONTAL);

    auto* staticText_2 = new wxStaticText(static_box_3->GetStaticBox(), wxID_ANY, "&Description:");
    flex_grid_sizer->Add(staticText_2, wxSizerFlags().Center().Border(wxALL));

    auto* description = new wxTextCtrl(static_box_3->GetStaticBox(), wxID_ANY, wxEmptyString);
    description->SetHint("Text");
    description->SetValidator(wxTextValidator(wxFILTER_NONE, &m_description));
    description->SetToolTip(
    "A short description of what the template is for. This string will be displayed in the file filter list of Windows file selectors. ");
    flex_grid_sizer->Add(description, wxSizerFlags().Expand().Border(wxALL));

    auto* staticText_10 = new wxStaticText(static_box_3->GetStaticBox(), wxID_ANY, "Class name:");
    staticText_10->SetToolTip("Change this to something unique to your project.");
    flex_grid_sizer->Add(staticText_10, wxSizerFlags().Center().Border(wxALL));

    auto* doc_classname = new wxTextCtrl(static_box_3->GetStaticBox(), wxID_ANY, "DocumentTextCtrlBase");
    doc_classname->SetValidator(wxTextValidator(wxFILTER_NONE, &m_doc_class));
    doc_classname->SetMinSize(ConvertDialogToPixels(wxSize(100, -1)));
    doc_classname->SetToolTip("Change this to something unique to your project.");
    flex_grid_sizer->Add(doc_classname, wxSizerFlags(1).Border(wxALL));

    auto* staticText_5 = new wxStaticText(static_box_3->GetStaticBox(), wxID_ANY, "&Extension:");
    flex_grid_sizer->Add(staticText_5, wxSizerFlags().Center().Border(wxALL));

    auto* extension = new wxTextCtrl(static_box_3->GetStaticBox(), wxID_ANY, wxEmptyString);
    extension->SetHint("txt");
    extension->SetValidator(wxTextValidator(wxFILTER_NONE, &m_default_extension));
    extension->SetToolTip("An appropriate file filter such as \"*.txt\". ");
    flex_grid_sizer->Add(extension, wxSizerFlags().Expand().Border(wxALL));

    auto* staticText_4 = new wxStaticText(static_box_3->GetStaticBox(), wxID_ANY, "&Filter:");
    flex_grid_sizer->Add(staticText_4, wxSizerFlags().Center().Border(wxALL));

    auto* filter = new wxTextCtrl(static_box_3->GetStaticBox(), wxID_ANY, wxEmptyString);
    filter->SetHint("*.txt");
    filter->SetValidator(wxTextValidator(wxFILTER_NONE, &m_filter));
    filter->SetToolTip("An appropriate file filter such as \"*.txt\". ");
    flex_grid_sizer->Add(filter, wxSizerFlags().Expand().Border(wxALL));

    static_box_3->Add(flex_grid_sizer, wxSizerFlags().Expand().Border(wxALL));

    box_sizer_3->Add(static_box_3, wxSizerFlags().Expand().Border(wxALL));

    box_sizer->Add(box_sizer_3, wxSizerFlags().Expand().Border(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    box_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    SetSizerAndFit(box_sizer);
    Centre(wxBOTH);

    // Event handlers
    Bind(wxEVT_BUTTON, &NewMdiForm::OnOK, this, wxID_OK);
    choice_view->Bind(wxEVT_CHOICE, &NewMdiForm::OnViewType, this);
    Bind(wxEVT_INIT_DIALOG,
        [](wxInitDialogEvent& event)
        {
            event.Skip();
        });
    folder_name->Bind(wxEVT_TEXT,
        [this](wxCommandEvent&)
        {VerifyClassName();
        });
    app_classname->Bind(wxEVT_TEXT,
        [this](wxCommandEvent&)
        {VerifyClassName();
        });
    view_classname->Bind(wxEVT_TEXT,
        [this](wxCommandEvent&)
        {VerifyClassName();
        });
    doc_classname->Bind(wxEVT_TEXT,
        [this](wxCommandEvent&)
        {VerifyClassName();
        });

    return true;
}

// ************* End of generated code ***********
// DO NOT EDIT THIS COMMENT BLOCK!
//
// Code below this comment block will be preserved
// if the code for this class is re-generated.
//
// clang-format on
// ***********************************************

/////////////////// Non-generated Copyright/License Info ////////////////////
// Purpose:   Dialog for creating a new MDI application
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

void NewMdiForm::createNode()
{
    auto folder = NodeCreation.createNode(gen_folder, nullptr);
    folder->set_value(prop_label, get_folder_name());
    auto form_node = NodeCreation.createNode(gen_DocViewApp, folder.get());
    ASSERT(form_node);
    folder->adoptChild(form_node);

    form_node->set_value(prop_class_name, get_app_class().utf8_string());
    if (form_node->as_string(prop_class_name) != form_node->getPropDefaultValue(prop_class_name))
    {
        UpdateFormClass(form_node.get());
    }
    if (m_view_type == "Text Control")
    {
        auto doc_node = NodeCreation.createNode(gen_DocumentTextCtrl, folder.get());
        ASSERT(doc_node);
        folder->adoptChild(doc_node);
        doc_node->set_value(prop_mdi_class_name, form_node->as_string(prop_class_name));
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

        auto frame_menu = NodeCreation.createNode(gen_MdiFrameMenuBar, doc_node.get());
        ASSERT(frame_menu);

        auto file_menu = NodeCreation.createNode(gen_wxMenu, frame_menu.get());
        ASSERT(file_menu);
        file_menu->set_value(prop_stock_id, "wxID_FILE");
        file_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_FILE")));

        auto menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_NEW");
        menu_item->set_value(prop_id, "wxID_NEW");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_NEW")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_NEW")));
        menu_item->set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_OPEN");
        menu_item->set_value(prop_id, "wxID_OPEN");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_OPEN")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_OPEN")));
        menu_item->set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_separator, file_menu.get());
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_EXIT");
        menu_item->set_value(prop_id, "wxID_EXIT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_EXIT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_EXIT")));
        menu_item->set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
        file_menu->adoptChild(menu_item);

        auto help_menu = NodeCreation.createNode(gen_wxMenu, frame_menu.get());
        ASSERT(help_menu);
        help_menu->set_value(prop_stock_id, "wxID_HELP");
        help_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_HELP")));

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_ABOUT");
        menu_item->set_value(prop_id, "wxID_ABOUT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_ABOUT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_ABOUT")));
        help_menu->adoptChild(menu_item);

        frame_menu->adoptChild(file_menu);
        frame_menu->adoptChild(help_menu);

        doc_node->adoptChild(frame_menu);
        auto doc_menu = NodeCreation.createNode(gen_MdiDocMenuBar, doc_node.get());
        ASSERT(doc_menu);
        file_menu = NodeCreation.createNode(gen_wxMenu, doc_menu.get());
        ASSERT(file_menu);
        file_menu->set_value(prop_stock_id, "wxID_FILE");
        file_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_FILE")));

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_NEW");
        menu_item->set_value(prop_id, "wxID_NEW");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_NEW")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_NEW")));
        menu_item->set_value(prop_bitmap, "Art;wxART_NEW|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_OPEN");
        menu_item->set_value(prop_id, "wxID_OPEN");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_OPEN")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_OPEN")));
        menu_item->set_value(prop_bitmap, "Art;wxART_FILE_OPEN|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_SAVE");
        menu_item->set_value(prop_id, "wxID_SAVE");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_SAVE")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_SAVE")));
        menu_item->set_value(prop_bitmap, "Art;wxART_FILE_SAVE|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_SAVEAS");
        menu_item->set_value(prop_id, "wxID_SAVEAS");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_SAVEAS")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_SAVEAS")));
        menu_item->set_value(prop_bitmap, "Art;wxART_FILE_SAVE_AS|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_separator, file_menu.get());
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_PRINT");
        menu_item->set_value(prop_id, "wxID_PRINT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_PRINT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_PRINT")));
        menu_item->set_value(prop_bitmap, "Art;wxART_PRINT|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_separator, file_menu.get());
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_CLOSE");
        menu_item->set_value(prop_id, "wxID_CLOSE");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_CLOSE")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_CLOSE")));
        menu_item->set_value(prop_bitmap, "Art;wxART_CLOSE|wxART_MENU");
        file_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_EXIT");
        menu_item->set_value(prop_id, "wxID_EXIT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_EXIT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_EXIT")));
        menu_item->set_value(prop_bitmap, "Art;wxART_QUIT|wxART_MENU");
        file_menu->adoptChild(menu_item);

        auto edit_menu = NodeCreation.createNode(gen_wxMenu, doc_menu.get());
        ASSERT(edit_menu);
        edit_menu->set_value(prop_stock_id, "wxID_EDIT");
        edit_menu->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_EDIT")));

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_CUT");
        menu_item->set_value(prop_id, "wxID_CUT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_CUT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_CUT")));
        menu_item->set_value(prop_bitmap, "Art;wxART_CUT|wxART_MENU");
        edit_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_COPY");
        menu_item->set_value(prop_id, "wxID_COPY");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_COPY")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_COPY")));
        menu_item->set_value(prop_bitmap, "Art;wxART_COPY|wxART_MENU");
        edit_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_PASTE");
        menu_item->set_value(prop_id, "wxID_PASTE");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_PASTE")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_PASTE")));
        menu_item->set_value(prop_bitmap, "Art;wxID_PASTE|wxART_MENU");
        edit_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_separator, file_menu.get());
        edit_menu->adoptChild(menu_item);

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_SELECTALL");
        menu_item->set_value(prop_id, "wxID_SELECTALL");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_SELECTALL")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_SELECTALL")));
        menu_item->set_value(prop_bitmap, "Art;wxART_PASTE|wxART_MENU");
        edit_menu->adoptChild(menu_item);

        help_menu = NodeCreation.createNode(gen_wxMenu, doc_menu.get());
        ASSERT(help_menu);
        help_menu->set_value(prop_label, "wxID_HELP");
        help_menu->set_value(prop_stock_id, "wxID_HELP");

        menu_item = NodeCreation.createNode(gen_wxMenuItem, file_menu.get());
        menu_item->set_value(prop_stock_id, "wxID_ABOUT");
        menu_item->set_value(prop_id, "wxID_ABOUT");
        menu_item->set_value(prop_label, wxGetStockLabel(NodeCreation.getConstantAsInt("wxID_ABOUT")));
        menu_item->set_value(prop_help, wxGetStockHelpString(NodeCreation.getConstantAsInt("wxID_ABOUT")));
        help_menu->adoptChild(menu_item);

        doc_menu->adoptChild(file_menu);
        doc_menu->adoptChild(edit_menu);
        doc_menu->adoptChild(help_menu);

        doc_node->adoptChild(doc_menu);
        auto view = NodeCreation.createNode(gen_ViewTextCtrl, folder.get());
        ASSERT(view);
        view->set_value(prop_mdi_doc_name, doc_node->as_string(prop_class_name));
        folder->adoptChild(view);
    }

    auto parent_node = wxGetFrame().GetSelectedNode();
    if (!parent_node)
    {
        parent_node = Project.getProjectNode();
    }
    else
    {
        parent_node = parent_node->getValidFormParent();
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
