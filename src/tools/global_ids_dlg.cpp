/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to Globally edit Custom IDs
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "global_ids_dlg_base.h"  // Generated header

#include "mainframe.h"        // MainFrame -- Main application window
#include "project_handler.h"  // ProjectHandler class
#include "undo_cmds.h"        // Undoable command classes derived from UndoAction

void MainFrame::OnEditCustomIds(wxCommandEvent& WXUNUSED(event))
{
    GlobalCustomIDS dlg(this);
    dlg.ShowModal();
}

void GlobalCustomIDS::OnInit(wxInitDialogEvent& event)
{
    m_lb_folders->Append("Project", Project.ProjectNode());
    for (const auto& iter: Project.ProjectNode()->GetChildNodePtrs())
    {
        if (iter->isGen(gen_folder))
        {
            m_lb_folders->Append(iter->as_string(prop_label), iter.get());
        }
        else if (iter->IsForm() && iter->HasValue(prop_class_name))
        {
            m_lb_forms->Append(iter->as_string(prop_class_name), iter.get());
        }
    }

    if (Project.ProjectNode()->HasValue(prop_id_prefixes))
    {
        for (auto& iter: Project.ProjectNode()->as_ArrayString(prop_id_prefixes))
        {
            m_combo_prefixes->Append(iter.make_wxString());
        }
    }

    if (Project.ProjectNode()->HasValue(prop_id_suffixes))
    {
        for (auto& iter: Project.ProjectNode()->as_ArrayString(prop_id_suffixes))
        {
            m_combo_suffixes->Append(iter.make_wxString());
        }
    }
    event.Skip();  // transfer all validator data to their windows and update UI
}

void GlobalCustomIDS::OnSelectFolders(wxCommandEvent& WXUNUSED(event))
{
    m_lb_forms->Clear();
    wxArrayInt selections;

    if (auto count = m_lb_folders->GetSelections(selections); count > 0)
    {
        for (auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_folders->GetClientData(iter));
            if (node)
            {
                for (const auto& form: node->GetChildNodePtrs())
                {
                    if (form->IsForm() && form->HasValue(prop_class_name))
                    {
                        m_lb_forms->Append(form->as_string(prop_class_name), form.get());
                    }
                    else if (form.get() == Project.ProjectNode())
                    {
                        m_lb_forms->Append("Project", form.get());
                    }
                }
            }
        }
    }
}

const int min_rows = 10;

struct NODE_IDS
{
    tt_string id_portion;
    Node* node;
};

void GlobalCustomIDS::OnSelectForms(wxCommandEvent& WXUNUSED(event))
{
    m_grid->ClearGrid();
    if (m_grid->GetNumberRows() > min_rows)
    {
        m_grid->DeleteRows(min_rows - 1, m_grid->GetNumberRows() - min_rows);
    }

    wxArrayInt selections;
    std::vector<NODE_IDS> ids;

    if (auto count = m_lb_forms->GetSelections(selections); count > 0)
    {
        // Collect all non "wx" IDs into the ids vector
        auto CollectIDs = [&](Node* node, auto&& CollectIDs) -> void
        {
            if (node->HasValue(prop_id) && !node->as_string(prop_id).is_sameprefix("wx"))
            {
                NODE_IDS node_id;
                node_id.id_portion = node->get_prop_id();
                node_id.node = node;
                ids.push_back(node_id);
            }

            for (const auto& iter: node->GetChildNodePtrs())
            {
                CollectIDs(iter.get(), CollectIDs);
            }
        };

        for (auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_forms->GetClientData(iter));
            CollectIDs(node, CollectIDs);
        }
    }

    // REVIEW: [Randalphwa - 07-10-2023] We could sort the ids vector here, but that could be
    // confusing since that would result in ids from different forms being mixed together
    // looking like they are duplicates.

    if (ids.size())
    {
        if (ids.size() > min_rows)
        {
            m_grid->AppendRows((to_int) ids.size() - min_rows);
        }

        int pos = 0;
        for (auto& iter: ids)
        {
            tt_string modified_id = iter.id_portion;
            if (m_text_old_prefix->GetValue().length())
            {
                auto old_prefix = m_text_old_prefix->GetValue().utf8_string();
                if (modified_id.starts_with(old_prefix))
                {
                    modified_id.erase(0, old_prefix.length());
                }
            }
            if (m_text_old_suffix->GetValue().length())
            {
                auto old_suffix = m_text_old_suffix->GetValue().utf8_string();
                if (modified_id.ends_with(old_suffix))
                {
                    modified_id.erase(modified_id.length() - old_suffix.length());
                }
            }
            if (m_combo_prefixes->GetValue().length())
            {
                modified_id.insert(0, m_combo_prefixes->GetValue().utf8_string());
            }
            if (m_combo_suffixes->GetValue().length())
            {
                modified_id.append(m_combo_suffixes->GetValue().utf8_string());
            }

            if (modified_id != iter.id_portion)
            {
                m_grid->SetCellValue(pos, 1, modified_id);
            }

            m_grid->SetCellValue(pos++, 0, iter.id_portion);
        }
    }
}

void GlobalCustomIDS::OnSelectAllFolders(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int idx = 0; idx < m_lb_folders->GetCount(); ++idx)
    {
        m_lb_folders->SetSelection(idx, true);
    }
}

void GlobalCustomIDS::OnSelectNoFolders(wxCommandEvent& WXUNUSED(event))
{
    m_lb_folders->DeselectAll();
}

void GlobalCustomIDS::OnSelectAllForms(wxCommandEvent& WXUNUSED(event))
{
    for (unsigned int idx = 0; idx < m_lb_folders->GetCount(); ++idx)
    {
        m_lb_forms->SetSelection(idx, true);
    }
}

void GlobalCustomIDS::OnSelectNoForms(wxCommandEvent& WXUNUSED(event))
{
    m_lb_forms->DeselectAll();
}

void GlobalCustomIDS::OnUpdate(wxCommandEvent& event)
{
    OnSelectForms(event);
}

void GlobalCustomIDS::OnCommit(wxCommandEvent& WXUNUSED(event))
{
    wxArrayInt selections;
    std::vector<NODE_IDS> ids;

    if (auto count = m_lb_forms->GetSelections(selections); count > 0)
    {
        // Collect all non "wx" IDs into the ids vector
        auto CollectIDs = [&](Node* node, auto&& CollectIDs) -> void
        {
            if (node->HasValue(prop_id) && !node->as_string(prop_id).is_sameprefix("wx"))
            {
                NODE_IDS node_id;
                node_id.id_portion = node->get_prop_id();
                node_id.node = node;
                ids.push_back(node_id);
            }

            for (const auto& iter: node->GetChildNodePtrs())
            {
                CollectIDs(iter.get(), CollectIDs);
            }
        };

        for (auto& iter: selections)
        {
            auto* node = static_cast<Node*>(m_lb_forms->GetClientData(iter));
            CollectIDs(node, CollectIDs);
        }
    }

    if (ids.empty())
    {
        wxMessageBox("No IDs to update", "Update IDs", wxOK | wxICON_INFORMATION);
        return;
    }

    auto undo_ids = std::make_shared<ModifyProperties>("Change IDs");

    for (auto& iter: ids)
    {
        tt_string modified_id = iter.id_portion;
        if (m_text_old_prefix->GetValue().length())
        {
            auto old_prefix = m_text_old_prefix->GetValue().utf8_string();
            if (modified_id.starts_with(old_prefix))
            {
                modified_id.erase(0, old_prefix.length());
            }
        }
        if (m_text_old_suffix->GetValue().length())
        {
            auto old_suffix = m_text_old_suffix->GetValue().utf8_string();
            if (modified_id.ends_with(old_suffix))
            {
                modified_id.erase(modified_id.length() - old_suffix.length());
            }
        }
        if (m_combo_prefixes->GetValue().length())
        {
            modified_id.insert(0, m_combo_prefixes->GetValue().utf8_string());
        }
        if (m_combo_suffixes->GetValue().length())
        {
            modified_id.append(m_combo_suffixes->GetValue().utf8_string());
        }

        if (modified_id != iter.id_portion)
        {
            tt_string new_id = iter.node->as_string(prop_id);
            new_id.Replace(iter.id_portion, modified_id);
            undo_ids->AddProperty(iter.node->get_prop_ptr(prop_id), new_id);
        }
    }

    wxGetFrame().PushUndoAction(undo_ids);

    m_text_old_prefix->Clear();
    m_text_old_suffix->Clear();
    m_combo_prefixes->SetValue("");
    m_combo_suffixes->SetValue("");

    wxCommandEvent dummy;
    OnUpdate(dummy);
}
