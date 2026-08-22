/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing a ID name and optional value
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-15-2026]

#include "id_editor_dlg.h"  // auto-generated: id_editor_dlg_base.h and id_editor_dlg_base.cpp

#include <algorithm>  // for std::sort
#include <tuple>      // for std::ignore

IDEditorDlg::IDEditorDlg(wxWindow* parent)
{
    std::ignore = Create(parent);
}

#include <wx/stockitem.h>  // stock items helpers

#include "wxue_namespace/wxue_string.h"  // wxue::string

#include "id_lists.h"

#include "node.h"             // Node class
#include "node_creator.h"     // NodeCreator -- Class used to create nodes
#include "project_handler.h"  // ProjectHandler class

void IDEditorDlg::OnInit(wxInitDialogEvent& event)
{
    m_standard_ids->Append(list_wx_ids);

    ASSERT_MSG(m_node, "You must call SetNode() before calling OnInit()")

    const std::vector<wxue::string> prefixes =
        Project.get_ProjectNode()->as_ArrayString(prop_id_prefixes);
    const std::vector<wxue::string> suffixes =
        Project.get_ProjectNode()->as_ArrayString(prop_id_suffixes);

    if (!prefixes.empty())
    {
        for (auto& iter: prefixes)
        {
            m_comboPrefixes->Append(iter);
        }
        m_comboPrefixes->SetSelection(0);
    }
    if (!suffixes.empty())
    {
        for (auto& iter: suffixes)
        {
            m_comboSuffix->Append(iter);
        }
        m_comboSuffix->SetSelection(0);
    }

    if (!prefixes.empty() || !suffixes.empty())
    {
        std::ignore = SelectPrefixSuffix(m_node->get_Form());
    }

    // Dummy event so that we can call event handlers (which don't actually use the event)
    wxCommandEvent dummy_event;

    // Make a copy of the id, because if it's a custom id, then we need to remove the prefix,
    // suffix and value.
    wxue::string cur_id = m_node->as_string(prop_id);
    if (cur_id.starts_with("wxID_"))
    {
        m_standard_ids->SetStringSelection(cur_id);
        OnStandardID(dummy_event);
    }
    else
    {
        if (auto pos = cur_id.find_first_of('='); pos != wxue::npos)
        {
            const wxue::string_view value = wxue::find_nonspace(cur_id.subview(pos + 1));
            m_textValue->SetValue(value.wx());
            cur_id.erase(pos);
            cur_id.trim();
        }

        std::vector<wxue::string> ordered_prefixes(prefixes);
        std::sort(ordered_prefixes.begin(), ordered_prefixes.end(),
                  [](const wxue::string& lhs, const wxue::string& rhs)
                  {
                      return lhs.size() > rhs.size();
                  });
        for (const auto& iter: ordered_prefixes)
        {
            if (cur_id.starts_with(iter))
            {
                m_comboPrefixes->SetStringSelection(iter);
                cur_id.erase(0, iter.size());
                m_checkAddPrefix->SetValue(true);
                break;
            }
        }
        std::vector<wxue::string> ordered_suffixes(suffixes);
        std::sort(ordered_suffixes.begin(), ordered_suffixes.end(),
                  [](const wxue::string& lhs, const wxue::string& rhs)
                  {
                      return lhs.size() > rhs.size();
                  });
        for (const auto& iter: ordered_suffixes)
        {
            if (cur_id.ends_with(iter))
            {
                m_comboSuffix->SetStringSelection(iter);
                cur_id.erase(cur_id.size() - iter.size());
                m_checkAddSuffix->SetValue(true);
                break;
            }
        }

        // The combos may have been pre-selected by SelectPrefixSuffix() from an unrelated node in
        // the form tree. Clear any selection whose checkbox is unchecked so a disabled combo can't
        // show a value that OnAffirmative() will not include.
        if (!m_checkAddPrefix->GetValue())
        {
            m_comboPrefixes->SetSelection(wxNOT_FOUND);
        }
        if (!m_checkAddSuffix->GetValue())
        {
            m_comboSuffix->SetSelection(wxNOT_FOUND);
        }

        m_textID->SetValue(cur_id);

        OnCustomID(dummy_event);
    }

    event.Skip();  // transfer all validator data to their windows and update UI
}

void IDEditorDlg::OnStdChange([[maybe_unused]] wxCommandEvent& event)
{
    m_result = m_standard_ids->GetStringSelection();
    m_final_id->SetLabel(m_result);

    for (auto& iter: list_stock_ids)
    {
        if (iter == m_result)
        {
            if (auto stock_id = NodeCreation.get_ConstantAsInt(iter); stock_id > 0)
            {
                m_flex_grid_sizer->ShowItems(true);
                m_stock_label->SetLabel(wxGetStockLabel(stock_id));
                m_help_text->SetLabel(wxGetStockHelpString(stock_id));
                Fit();
                return;
            }
        }
    }
    m_flex_grid_sizer->ShowItems(false);
    Fit();
}

void IDEditorDlg::OnStandardID(wxCommandEvent& event)
{
    m_radioBtn_Custom->SetValue(false);
    m_cstm_id_box->GetStaticBox()->Enable(false);

    m_std_id_box->GetStaticBox()->Enable(true);
    m_radioBtn_Standard->SetValue(true);

    OnComboSelect(event);
}

void IDEditorDlg::OnCustomID(wxCommandEvent& event)
{
    m_radioBtn_Custom->SetValue(true);
    m_cstm_id_box->GetStaticBox()->Enable(true);

    m_std_id_box->GetStaticBox()->Enable(false);
    m_radioBtn_Standard->SetValue(false);

    m_comboPrefixes->Enable(m_checkAddPrefix->GetValue());
    m_comboSuffix->Enable(m_checkAddSuffix->GetValue());

    OnComboSelect(event);
}

wxString IDEditorDlg::BuildCompleteId() const
{
    if (m_radioBtn_Standard->GetValue())
    {
        return m_standard_ids->GetStringSelection();
    }

    wxString complete_id;
    if (m_checkAddPrefix->GetValue())
    {
        complete_id << m_comboPrefixes->GetStringSelection();
    }

    complete_id << m_textID->GetValue();

    if (m_checkAddSuffix->GetValue())
    {
        complete_id << m_comboSuffix->GetStringSelection();
    }

    if (!m_textValue->GetValue().empty())
    {
        complete_id << " = " << m_textValue->GetValue();
    }

    return complete_id;
}

void IDEditorDlg::OnComboSelect([[maybe_unused]] wxCommandEvent& event)
{
    m_final_id->SetLabel(BuildCompleteId());
}

void IDEditorDlg::OnAffirmative(wxUpdateUIEvent& event)
{
    m_result = BuildCompleteId();

    event.Skip();
}

[[nodiscard]] bool IDEditorDlg::SelectPrefixSuffix(Node* node)
{
    if (!node)
    {
        return false;
    }

    if (node->HasProp(prop_id))
    {
        const wxue::string& id = node->as_string(prop_id);
        if (!id.starts_with("wxID_"))
        {
            if (!m_prefix_selected)
            {
                const std::vector<wxue::string> prefixes =
                    Project.get_ProjectNode()->as_ArrayString(prop_id_prefixes);
                for (const auto& iter: prefixes)
                {
                    if (id.starts_with(iter))
                    {
                        m_comboPrefixes->SetStringSelection(iter);
                        m_prefix_selected = true;
                        break;
                    }
                }
            }
            if (!m_suffix_selected)
            {
                const std::vector<wxue::string> suffixes =
                    Project.get_ProjectNode()->as_ArrayString(prop_id_suffixes);
                for (const auto& iter: suffixes)
                {
                    if (id.ends_with(iter))
                    {
                        m_comboSuffix->SetStringSelection(iter);
                        m_suffix_selected = true;
                        break;
                    }
                }
            }

            if (m_prefix_selected && m_suffix_selected)
            {
                return true;
            }
        }
    }

    for (const auto& iter: node->get_ChildNodePtrs())
    {
        if (SelectPrefixSuffix(iter.get()))
        {
            return true;
        }
    }

    return false;
}
