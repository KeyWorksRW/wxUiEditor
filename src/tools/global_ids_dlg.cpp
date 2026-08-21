/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to Globally edit Custom IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-21-2026]

#include "mainframe.h"        // MainFrame -- Main application window
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction

#include "wxue_namespace/wxue_string.h"  // wxue::string class

#include "global_ids_dlg.h"  // auto-generated: global_ids_dlg_base.h and global_ids_dlg_base.cpp

// If this constructor is used, the caller must call Create(parent)
GlobalCustomIDS::GlobalCustomIDS() {}

GlobalCustomIDS::GlobalCustomIDS(wxWindow* parent)
{
    std::ignore = Create(parent);
}

bool GlobalCustomIDS::Create(wxWindow* parent, wxWindowID id, const wxString& title,
                             const wxPoint& pos, const wxSize& size, long flags,
                             const wxString& name)
{
    if (!GlobalCustomIDSBase::Create(parent, id, title, pos, size, flags, name))
    {
        return false;
    }

    // The generated base only binds wxEVT_BUTTON to OnClose for wxID_CLOSE, but the
    // dialog's CreateStdDialogButtonSizer(wxCLOSE) generates a wxID_CANCEL button, and
    // the title-bar X sends wxEVT_CLOSE_WINDOW. Bind those so the "Commit changes?"
    // prompt actually fires when the user closes the dialog.
    Bind(wxEVT_BUTTON, &GlobalCustomIDS::OnClose, this, wxID_CANCEL);
    Bind(wxEVT_CLOSE_WINDOW, &GlobalCustomIDS::OnCloseWindow, this);

    return true;
}

void MainFrame::OnEditCustomIds([[maybe_unused]] wxCommandEvent& event)
{
    GlobalCustomIDS custom_ids_dlg(this);
    custom_ids_dlg.ShowModal();
}

void GlobalCustomIDS::OnInit(wxInitDialogEvent& event)
{
    m_lb_folders->Append("Project", Project.get_ProjectNode());
    for (const auto& iter: Project.get_ProjectNode()->get_ChildNodePtrs())
    {
        if (iter->is_Folder())
        {
            m_lb_folders->Append(iter->as_string(prop_label), iter.get());
        }
        else if (iter->is_Form() && iter->HasValue(prop_class_name))
        {
            m_lb_forms->Append(iter->as_string(prop_class_name), iter.get());
        }
    }

    if (Project.get_ProjectNode()->HasValue(prop_id_prefixes))
    {
        for (const auto& iter: Project.get_ProjectNode()->as_ArrayString(prop_id_prefixes))
        {
            m_combo_prefixes->Append(iter.wx());
        }
    }

    if (Project.get_ProjectNode()->HasValue(prop_id_suffixes))
    {
        for (const auto& iter: Project.get_ProjectNode()->as_ArrayString(prop_id_suffixes))
        {
            m_combo_suffixes->Append(iter.wx());
        }
    }
    event.Skip();  // transfer all validator data to their windows and update UI
}

void GlobalCustomIDS::OnSelectFolders([[maybe_unused]] wxCommandEvent& event)
{
    m_lb_forms->Clear();
    wxArrayInt selections;

    if (auto count = m_lb_folders->GetSelections(selections); count > 0)
    {
        // Recursively collect forms from the selected folder and all sub-folders.
        auto CollectForms = [&](Node* node, auto&& CollectForms) -> void
        {
            for (const auto& form: node->get_ChildNodePtrs())
            {
                if (form->is_Form() && form->HasValue(prop_class_name))
                {
                    m_lb_forms->Append(form->as_string(prop_class_name), form.get());
                }
                else if (form.get() == Project.get_ProjectNode())
                {
                    m_lb_forms->Append("Project", form.get());
                }
                else if (form->is_Folder())
                {
                    CollectForms(form.get(), CollectForms);
                }
            }
        };

        for (const auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_folders->GetClientData(iter));
            if (node)
            {
                CollectForms(node, CollectForms);
            }
        }
    }
}

const int min_rows = 10;

struct NODE_IDS
{
    wxue::string id_portion;
    Node* node;
};

void GlobalCustomIDS::OnSelectForms([[maybe_unused]] wxCommandEvent& event)
{
    m_grid->ClearGrid();
    if (m_grid->GetNumberRows() > min_rows)
    {
        m_grid->DeleteRows(min_rows - 1, m_grid->GetNumberRows() - min_rows);
    }

    wxArrayInt selections;
    std::vector<NODE_IDS> node_ids;

    if (auto count = m_lb_forms->GetSelections(selections); count > 0)
    {
        // Collect all non "wx" IDs into the ids vector
        auto CollectIDs = [&](Node* node, auto&& CollectIDs) -> void
        {
            if (node->HasValue(prop_id) && !node->as_string(prop_id).is_sameprefix("wx"))
            {
                NODE_IDS node_id;
                node_id.id_portion = node->get_PropId();
                node_id.node = node;
                node_ids.push_back(node_id);
            }

            for (const auto& iter: node->get_ChildNodePtrs())
            {
                CollectIDs(iter.get(), CollectIDs);
            }
        };

        for (const auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_forms->GetClientData(iter));
            CollectIDs(node, CollectIDs);
        }
    }

    // REVIEW: [Randalphwa - 07-10-2023] We could sort the ids vector here, but that could be
    // confusing since that would result in ids from different forms being mixed together
    // looking like they are duplicates.

    if (!node_ids.empty())
    {
        if (node_ids.size() > min_rows)
        {
            m_grid->AppendRows((to_int) node_ids.size() - min_rows);
        }

        int pos = 0;
        for (auto& iter: node_ids)
        {
            wxue::string modified_id = iter.id_portion;
            if (!m_text_old_prefix->GetValue().empty())
            {
                const std::string old_prefix = m_text_old_prefix->GetValue().utf8_string();
                if (modified_id.starts_with(old_prefix))
                {
                    modified_id.erase(0, old_prefix.length());
                }
            }
            if (!m_text_old_suffix->GetValue().empty())
            {
                const std::string old_suffix = m_text_old_suffix->GetValue().utf8_string();
                if (modified_id.ends_with(old_suffix))
                {
                    modified_id.erase(modified_id.length() - old_suffix.length());
                }
            }
            if (!m_combo_prefixes->GetValue().empty())
            {
                modified_id.insert(0, m_combo_prefixes->GetValue().utf8_string());
            }
            if (!m_combo_suffixes->GetValue().empty())
            {
                modified_id.append(m_combo_suffixes->GetValue().utf8_string());
            }

            if (modified_id != iter.id_portion)
            {
                m_grid->SetCellValue(pos, 1, modified_id);
                m_committed = false;
            }

            m_grid->SetCellValue(pos++, 0, iter.id_portion);
        }
    }
}

void GlobalCustomIDS::OnSelectAllFolders([[maybe_unused]] wxCommandEvent& event)
{
    for (unsigned int idx = 0; idx < m_lb_folders->GetCount(); ++idx)
    {
        m_lb_folders->SetSelection(idx, true);
    }
}

void GlobalCustomIDS::OnSelectNoFolders([[maybe_unused]] wxCommandEvent& event)
{
    m_lb_folders->DeselectAll();
}

void GlobalCustomIDS::OnSelectAllForms([[maybe_unused]] wxCommandEvent& event)
{
    for (unsigned int idx = 0; idx < m_lb_forms->GetCount(); ++idx)
    {
        m_lb_forms->SetSelection(idx, true);
    }
}

void GlobalCustomIDS::OnSelectNoForms([[maybe_unused]] wxCommandEvent& event)
{
    m_lb_forms->DeselectAll();
}

void GlobalCustomIDS::OnUpdate(wxCommandEvent& event)
{
    OnSelectForms(event);
}

void GlobalCustomIDS::OnCommit([[maybe_unused]] wxCommandEvent& event)
{
    wxArrayInt selections;
    std::vector<NODE_IDS> node_ids;

    if (auto count = m_lb_forms->GetSelections(selections); count > 0)
    {
        // Collect all non "wx" IDs into the ids vector
        auto CollectIDs = [&](Node* node, auto&& CollectIDs) -> void
        {
            if (node->HasValue(prop_id) && !node->as_string(prop_id).is_sameprefix("wx"))
            {
                NODE_IDS node_id;
                node_id.id_portion = node->get_PropId();
                node_id.node = node;
                node_ids.push_back(node_id);
            }

            for (const auto& iter: node->get_ChildNodePtrs())
            {
                CollectIDs(iter.get(), CollectIDs);
            }
        };

        for (const auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_forms->GetClientData(iter));
            CollectIDs(node, CollectIDs);
        }
    }

    if (node_ids.empty())
    {
        wxMessageBox("No IDs to update", "Update IDs", wxOK | wxICON_INFORMATION);
        return;
    }

    auto undo_ids = std::make_shared<ModifyProperties>("Change IDs");

    for (auto& iter: node_ids)
    {
        wxue::string modified_id = iter.id_portion;
        if (!m_text_old_prefix->GetValue().empty())
        {
            const std::string old_prefix = m_text_old_prefix->GetValue().utf8_string();
            if (modified_id.starts_with(old_prefix))
            {
                modified_id.erase(0, old_prefix.length());
            }
        }
        if (!m_text_old_suffix->GetValue().empty())
        {
            const std::string old_suffix = m_text_old_suffix->GetValue().utf8_string();
            if (modified_id.ends_with(old_suffix))
            {
                modified_id.erase(modified_id.length() - old_suffix.length());
            }
        }
        if (!m_combo_prefixes->GetValue().empty())
        {
            modified_id.insert(0, m_combo_prefixes->GetValue().utf8_string());
        }
        if (!m_combo_suffixes->GetValue().empty())
        {
            modified_id.append(m_combo_suffixes->GetValue().utf8_string());
        }

        if (modified_id != iter.id_portion)
        {
            wxue::string new_id = iter.node->as_string(prop_id);
            std::ignore = new_id.Replace(iter.id_portion, modified_id);
            undo_ids->addProperty(iter.node->get_PropPtr(prop_id), new_id);
        }
    }

    if (undo_ids->GetVector().empty())
    {
        // Nothing actually changed (e.g. the old prefix/suffix didn't match anything,
        // or the IDs already have the new prefix). Don't dirty the project with an
        // empty undo action.
        wxMessageBox("No changes were made", "Update IDs", wxOK | wxICON_INFORMATION);
        return;
    }

    wxGetFrame().PushUndoAction(undo_ids);

    m_text_old_prefix->Clear();
    m_text_old_suffix->Clear();
    m_combo_prefixes->SetValue("");
    m_combo_suffixes->SetValue("");

    m_committed = true;

    wxCommandEvent dummy;
    OnUpdate(dummy);
}

void GlobalCustomIDS::CommitIfNotCommitted()
{
    if (m_committed)
    {
        return;
    }

    if (wxMessageBox("Commit changes?", "Update IDs", wxYES_NO | wxICON_QUESTION) == wxYES)
    {
        wxCommandEvent commit_event;
        OnCommit(commit_event);
    }
}

void GlobalCustomIDS::OnClose(wxCommandEvent& event)
{
    CommitIfNotCommitted();
    event.Skip();
}

void GlobalCustomIDS::OnCloseWindow(wxCloseEvent& event)
{
    CommitIfNotCommitted();
    event.Skip();
}
