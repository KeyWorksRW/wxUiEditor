///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "new_dialog.h"

bool NewDialog::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* parent_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto* staticText_3 = new wxStaticText(this, wxID_ANY,
        "These are initial values -- all of them can be changed after the dialog is created.");
    staticText_3->Wrap(300);
    box_sizer_3->Add(staticText_3, wxSizerFlags().Border(wxALL));

    m_infoBar = new wxInfoBar(this);
    m_infoBar->SetShowHideEffects(wxSHOW_EFFECT_NONE, wxSHOW_EFFECT_NONE);
    m_infoBar->SetEffectDuration(500);
    box_sizer_3->Add(m_infoBar, wxSizerFlags().Expand().Border(wxALL));

    parent_sizer->Add(box_sizer_3, wxSizerFlags().Border(wxALL));

    auto* box_sizer_2 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText = new wxStaticText(this, wxID_ANY, "&Base class name:");
    staticText->SetToolTip("Change this to something unique to your project.");
    box_sizer_2->Add(staticText, wxSizerFlags().Center().Border(wxALL));

    m_classname = new wxTextCtrl(this, wxID_ANY, "MyDialogBase");
    m_classname->SetValidator(wxTextValidator(wxFILTER_NONE, &m_base_class));
    m_classname->SetToolTip("Change this to something unique to your project.");
    box_sizer_2->Add(m_classname, wxSizerFlags(1).Border(wxALL));

    parent_sizer->Add(box_sizer_2, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer__2 = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText_2 = new wxStaticText(this, wxID_ANY, "&Title:");
    box_sizer__2->Add(staticText_2, wxSizerFlags().Center().Border(wxALL));

    m_textCtrl_title = new wxTextCtrl(this, wxID_ANY, wxEmptyString);
    m_textCtrl_title->SetHint("Text for dialog\'s Title Bar");
    m_textCtrl_title->SetValidator(wxTextValidator(wxFILTER_NONE, &m_title));
    box_sizer__2->Add(m_textCtrl_title, wxSizerFlags(1).Border(wxALL));

    parent_sizer->Add(box_sizer__2, wxSizerFlags().Expand().Border(wxALL));

    auto* box_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_4 = new wxBoxSizer(wxHORIZONTAL);

    m_check_tabs = new wxCheckBox(this, wxID_ANY, "Tabbed &Dialog");
    m_check_tabs->SetValidator(wxGenericValidator(&m_has_tabs));
    m_check_tabs->SetToolTip("If checked, creates a wxNotebook");
    box_sizer_4->Add(m_check_tabs, wxSizerFlags().Center().Border(wxALL));

    auto* staticText_4 = new wxStaticText(this, wxID_ANY, "Tab&s:");
    box_sizer_4->Add(staticText_4, wxSizerFlags().Center().Border(wxLEFT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    m_spinCtrlTabs = new wxSpinCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 7, 3);
    m_spinCtrlTabs->SetValidator(wxGenericValidator(&m_num_tabs));
    m_spinCtrlTabs->Enable(false);
    box_sizer_4->Add(m_spinCtrlTabs, wxSizerFlags().Center().Border(wxALL));

    box_sizer->Add(box_sizer_4, wxSizerFlags().Border(wxRIGHT|wxTOP|wxBOTTOM, wxSizerFlags::GetDefaultBorder()));

    auto* checkBox_2 = new wxCheckBox(this, wxID_ANY, "&Standard Buttons");
    checkBox_2->SetValue(true);
    checkBox_2->SetValidator(wxGenericValidator(&m_has_std_btns));
    box_sizer->Add(checkBox_2, wxSizerFlags().Border(wxALL));

    parent_sizer->Add(box_sizer, wxSizerFlags().Border(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    parent_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    if (pos != wxDefaultPosition)
    {
        SetPosition(FromDIP(pos));
    }
    if (size == wxDefaultSize)
    {
        SetSizerAndFit(parent_sizer);
    }
    else
    {
        SetSizer(parent_sizer);
        if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
        {
            Fit();
        }
        SetSize(FromDIP(size));
        Layout();
    }
    Centre(wxBOTH);

    // Event handlers
    m_check_tabs->Bind(wxEVT_CHECKBOX,
        [this](wxCommandEvent&)
        {
            m_spinCtrlTabs->Enable(m_check_tabs->GetValue());
        });
    Bind(wxEVT_INIT_DIALOG, &NewDialog::OnInit, this);
    m_classname->Bind(wxEVT_TEXT,
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
// Purpose:   Dialog for creating a new project dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

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

void NewDialog::createNode()
{
    auto form_node = NodeCreation.createNode(gen_wxDialog, nullptr).first;
    ASSERT(form_node);

    if (m_title.size())
    {
        form_node->set_value(prop_title, m_title.utf8_string());
    }

    auto parent_sizer = NodeCreation.createNode(gen_VerticalBoxSizer, form_node.get()).first;
    ASSERT(parent_sizer);
    parent_sizer->set_value(prop_var_name, "dlg_sizer");
    form_node->adoptChild(parent_sizer);

    if (m_has_tabs)
    {
        auto notebook = NodeCreation.createNode(gen_wxNotebook, parent_sizer.get()).first;
        ASSERT(notebook);
        parent_sizer->adoptChild(notebook);

        for (int count = 0; count < m_num_tabs; ++count)
        {
            auto book_page = NodeCreation.createNode(gen_BookPage, notebook.get()).first;
            notebook->adoptChild(book_page);

            tt_string label("Tab ");
            label << count + 1;
            book_page->set_value(prop_label, label);
            auto page_sizer = NodeCreation.createNode(gen_VerticalBoxSizer, book_page.get()).first;
            page_sizer->set_value(prop_var_name, tt_string() << "page_sizer_" << count + 1);
            book_page->adoptChild(page_sizer);
            auto static_text = NodeCreation.createNode(gen_wxStaticText, page_sizer.get()).first;
            page_sizer->adoptChild(static_text);
            static_text->set_value(prop_label, "TODO: replace this control with something more useful...");
            static_text->set_value(prop_wrap, "200");
        }
    }

    if (m_has_std_btns)
    {
        auto std_btn = NodeCreation.createNode(gen_wxStdDialogButtonSizer, parent_sizer.get()).first;
        parent_sizer->adoptChild(std_btn);

        std_btn->set_value(prop_OK, "1");
        std_btn->set_value(prop_Cancel, "1");
        std_btn->set_value(prop_static_line, "1");
        std_btn->set_value(prop_default_button, "OK");
        std_btn->set_value(prop_flags, "wxEXPAND");
    }

    form_node->set_value(prop_class_name, m_base_class.utf8_string());
    if (form_node->as_string(prop_class_name) != form_node->getPropDefaultValue(prop_class_name))
    {
        UpdateFormClass(form_node.get());
    }

    auto parent_node = wxGetFrame().getSelectedNode();
    if (!parent_node)
    {
        parent_node = Project.getProjectNode();
    }
    else
    {
        parent_node = parent_node->getValidFormParent();
    }

    wxGetFrame().SelectNode(parent_node);

    tt_string undo_str("New wxDialog");
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().getNavigationPanel()->ChangeExpansion(form_node.get(), true, true);
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
