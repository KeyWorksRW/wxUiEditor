/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for creating a new wizard
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "wxui/newwizard_base.h"  // auto-generated: wxui/newwizard_base.h and wxui/newwizard_base.cpp

#include "../panels/nav_panel.h"  // NavigationPanel -- Navigation Panel
#include "mainframe.h"            // MainFrame -- Main window frame
#include "new_common.h"           // Contains code common between all new_ dialogs
#include "node.h"                 // Node class
#include "node_creator.h"         // NodeCreator -- Class used to create nodes
#include "project_handler.h"      // ProjectHandler class
#include "undo_cmds.h"            // InsertNodeAction -- Undoable command classes derived from UndoAction

void NewWizard::OnInit(wxInitDialogEvent& event)
{
    // BUGBUG: [KeyWorks - 01-13-2022] Remove once issue #616 is fixed
    m_classname->SetFocus();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void NewWizard::CreateNode()
{
    auto new_node = NodeCreation.CreateNode(gen_wxWizard, nullptr);
    ASSERT(new_node);

    if (m_title.size())
    {
        new_node->set_value(prop_title, m_title.utf8_string());
    }

    for (int count = 0; count < m_num_pages; ++count)
    {
        if (auto page = NodeCreation.CreateNode(gen_wxWizardPageSimple, new_node.get()); page)
        {
            page->set_value(prop_var_name, tt_string("wizard_page_") << count + 1);
            auto sizer = NodeCreation.CreateNode(gen_VerticalBoxSizer, page.get());

            auto static_text = NodeCreation.CreateNode(gen_wxStaticText, sizer.get());
            static_text->set_value(prop_class_access, "none");
            static_text->set_value(prop_var_name, tt_string("static_text_") << count + 1);
            sizer->Adopt(static_text);
            static_text->set_value(prop_label, tt_string("Page #")
                                                   << count + 1
                                                   << " -- TODO: replace this control with something more useful...");
            static_text->set_value(prop_wrap, "200");

            page->Adopt(sizer);
            new_node->Adopt(page);
        }
    }
    new_node->FixDuplicateNodeNames();

    new_node->set_value(prop_class_name, m_base_class.utf8_string());
    if (new_node->as_string(prop_class_name) != new_node->prop_default_value(prop_class_name))
    {
        UpdateFormClass(new_node.get());
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

    tt_string undo_str("New wxWizard");
    wxGetFrame().PushUndoAction(std::make_shared<InsertNodeAction>(new_node.get(), parent_node, undo_str, -1));
    wxGetFrame().FireCreatedEvent(new_node);
    wxGetFrame().SelectNode(new_node, evt_flags::fire_event | evt_flags::force_selection);
    wxGetFrame().GetNavigationPanel()->ChangeExpansion(new_node.get(), true, true);
}

// Called whenever m_classname changes
void NewWizard::VerifyClassName()
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
