///////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor - see https://github.com/KeyWorksRW/wxUiEditor/
//
// Do not edit any code above the "End of generated code" comment block.
// Any changes before that block will be lost if it is re-generated!
///////////////////////////////////////////////////////////////////////////////

// clang-format off

#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/valgen.h>
#include <wx/valtext.h>

#include "new_frame.h"

bool NewFrame::Create(wxWindow* parent, wxWindowID id, const wxString& title,
    const wxPoint& pos, const wxSize& size, long style, const wxString &name)
{
    if (!wxDialog::Create(parent, id, title, pos, size, style, name))
        return false;

    auto* box_sizer = new wxBoxSizer(wxVERTICAL);

    auto* box_sizer_3 = new wxBoxSizer(wxVERTICAL);

    auto* staticText_3 = new wxStaticText(this, wxID_ANY,
        "These are initial values -- all of them can be changed after the window is created.");
    staticText_3->Wrap(300);
    box_sizer_3->Add(staticText_3, wxSizerFlags().Border(wxALL));

    m_infoBar = new wxInfoBar(this);
    m_infoBar->SetShowHideEffects(wxSHOW_EFFECT_NONE, wxSHOW_EFFECT_NONE);
    m_infoBar->SetEffectDuration(500);
    box_sizer_3->Add(m_infoBar, wxSizerFlags().Expand().Border(wxALL));

    box_sizer->Add(box_sizer_3, wxSizerFlags().Border(wxALL));

    auto* class_sizer = new wxBoxSizer(wxHORIZONTAL);

    auto* staticText = new wxStaticText(this, wxID_ANY, "&Base class name:");
    class_sizer->Add(staticText, wxSizerFlags().Center().Border(wxALL));

    m_classname = new wxTextCtrl(this, wxID_ANY, "MyFrameBase");
    m_classname->SetValidator(wxTextValidator(wxFILTER_NONE, &m_base_class));
    class_sizer->Add(m_classname, wxSizerFlags(1).Border(wxALL));

    box_sizer->Add(class_sizer, wxSizerFlags().Expand().Border(wxALL));

    m_checkBox_mainframe = new wxCheckBox(this, wxID_ANY, "Main Frame Window");
    m_checkBox_mainframe->SetValidator(wxGenericValidator(&m_has_mainframe));

    auto* static_box = new wxStaticBoxSizer(new wxStaticBox(this, wxID_ANY, m_checkBox_mainframe), wxVERTICAL);

    auto* box_sizer_2 = new wxBoxSizer(wxVERTICAL);

    m_checkBox_toolbar = new wxCheckBox(static_box->GetStaticBox(), wxID_ANY, "Add Toolbar");
    m_checkBox_toolbar->SetValue(true);
    m_checkBox_toolbar->SetValidator(wxGenericValidator(&m_has_toolbar));
    box_sizer_2->Add(m_checkBox_toolbar, wxSizerFlags().Expand().Border(wxALL));

    m_checkBox_menu = new wxCheckBox(static_box->GetStaticBox(), wxID_ANY, "Add Menu");
    m_checkBox_menu->SetValue(true);
    m_checkBox_menu->SetValidator(wxGenericValidator(&m_has_menu));
    box_sizer_2->Add(m_checkBox_menu, wxSizerFlags().Expand().Border(wxALL));

    m_checkBox_statusbar = new wxCheckBox(static_box->GetStaticBox(), wxID_ANY, "Add Statusbar");
    m_checkBox_statusbar->SetValue(true);
    m_checkBox_statusbar->SetValidator(wxGenericValidator(&m_has_statusbar));
    box_sizer_2->Add(m_checkBox_statusbar, wxSizerFlags().Expand().Border(wxALL));

    static_box->Add(box_sizer_2, wxSizerFlags().Expand().DoubleBorder(wxALL));

    box_sizer->Add(static_box, wxSizerFlags().Expand().DoubleBorder(wxALL));

    auto* stdBtn = CreateStdDialogButtonSizer(wxOK|wxCANCEL);
    box_sizer->Add(CreateSeparatedSizer(stdBtn), wxSizerFlags().Expand().Border(wxALL));

    if (pos != wxDefaultPosition)
    {
        SetPosition(FromDIP(pos));
    }
    if (size == wxDefaultSize)
    {
        SetSizerAndFit(box_sizer);
    }
    else
    {
        SetSizer(box_sizer);
        if (size.x == wxDefaultCoord || size.y == wxDefaultCoord)
        {
            Fit();
        }
        SetSize(FromDIP(size));
        Layout();
    }
    Centre(wxBOTH);

    // Event handlers
    m_checkBox_mainframe->Bind(wxEVT_CHECKBOX, &NewFrame::OnCheckMainFrame, this);
    Bind(wxEVT_INIT_DIALOG,
        [this](wxInitDialogEvent& event)
        {
            m_classname->SetFocus();
            event.Skip();
        });
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
// Purpose:   Dialog for creating a new project wxFrame
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "../panels/nav_panel.h"     // NavigationPanel -- Navigation Panel
#include "../panels/ribbon_tools.h"  // RibbonPanel -- Displays component tools in a wxRibbonBar
#include "mainframe.h"               // MoveDirection -- Main window frame
#include "new_common.h"              // Contains code common between all new_ dialogs
#include "node.h"                    // Node class
#include "node_creator.h"            // NodeCreator -- Class used to create nodes
#include "project_handler.h"         // ProjectHandler class
#include "undo_cmds.h"               // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewFrame::OnCheckMainFrame(wxCommandEvent& WXUNUSED(event))
{
    if (m_checkBox_mainframe->GetValue())
    {
        m_checkBox_toolbar->Enable();
        m_checkBox_menu->Enable();
        m_checkBox_statusbar->Enable();
    }
    else
    {
        m_checkBox_toolbar->Disable();
        m_checkBox_menu->Disable();
        m_checkBox_statusbar->Disable();
    }
}

void NewFrame::createNode()
{
    auto form_node = NodeCreation.createNode(gen_wxFrame, nullptr).first;
    ASSERT(form_node);

    if (m_has_mainframe)
    {
        if (m_has_toolbar)
        {
            auto bar = NodeCreation.createNode(gen_wxToolBar, form_node.get()).first;
            ASSERT(bar);
            form_node->adoptChild(bar);
        }
        if (m_has_menu)
        {
            auto bar = NodeCreation.createNode(gen_wxMenuBar, form_node.get()).first;
            ASSERT(bar);
            form_node->adoptChild(bar);
        }
        if (m_has_statusbar)
        {
            auto bar = NodeCreation.createNode(gen_wxStatusBar, form_node.get()).first;
            ASSERT(bar);
            form_node->adoptChild(bar);
        }
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

    tt_string undo_str("New wxFrame");
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(form_node.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(form_node);
    wxGetFrame().SelectNode(form_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().getNavigationPanel()->ChangeExpansion(form_node.get(), true, true);

    // If it's a mainframe then bars were probably added, so it makes sense to switch to the Bars ribbon bar page since
    // that's likely what the user will be doing next (adding tools or menus).
    if (m_has_mainframe)
        wxGetFrame().getRibbonPanel()->ActivateBarPage();
}

// Called whenever m_classname changes
void NewFrame::VerifyClassName()
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
