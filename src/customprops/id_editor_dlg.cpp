/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for editing a ID name and optional value
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [11-03-2026]

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
#include "node_prop.h"        // NodeProperty class
#include "project_handler.h"  // ProjectHandler class

constexpr wxWindowID s_make_unique_id { wxID_HIGHEST + 1 };

void IDEditorDlg::OnInit(wxInitDialogEvent& event)
{
    m_standard_ids->Append(list_wx_ids);

    ASSERT_MSG(m_node, "You must call SetNode() before calling OnInit()")

    // The info bar button must exist before any of the SetValue() calls below, since those fire
    // wxEVT_TEXT which updates the duplicate-ID warning.
    CollectFormIds();
    m_infoBar->AddButton(s_make_unique_id, "Make &Unique");
    m_infoBar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &IDEditorDlg::OnMakeUnique, this,
                    s_make_unique_id);

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

    UpdateDuplicateIdWarning();

    event.Skip();  // transfer all validator data to their windows and update UI
}

void IDEditorDlg::OnStdChange([[maybe_unused]] wxCommandEvent& event)
{
    UpdateDuplicateIdWarning();

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

    wxString complete_id = BuildIdName(m_textID->GetValue());

    if (!m_textValue->GetValue().empty())
    {
        complete_id << " = " << m_textValue->GetValue();
    }

    return complete_id;
}

wxString IDEditorDlg::BuildIdName(const wxString& id_part) const
{
    wxString id_name;
    if (m_checkAddPrefix->GetValue())
    {
        id_name << m_comboPrefixes->GetStringSelection();
    }

    id_name << id_part;

    if (m_checkAddSuffix->GetValue())
    {
        id_name << m_comboSuffix->GetStringSelection();
    }

    return id_name;
}

void IDEditorDlg::CollectFormIds()
{
    m_form_ids.clear();

    // Mirror BaseGenerator::AllowIdPropertyChange() -- the scope is the form, except for nodes that
    // live in a toolbar, menubar, or ribbon bar where the bar itself is the scope.
    Node* scope_node = m_node->get_Form();
    if (m_node->is_Gen(gen_wxMenuItem))
    {
        scope_node = m_node->get_Parent();
        while (scope_node && !scope_node->is_Gen(gen_wxMenuBar) && !scope_node->is_Gen(gen_MenuBar))
        {
            scope_node = scope_node->get_Parent();
        }
    }
    else if (m_node->is_Gen(gen_auitool))
    {
        scope_node = m_node->get_Parent();
        while (scope_node && !scope_node->is_Gen(gen_AuiToolBar) &&
               !scope_node->is_Gen(gen_wxAuiToolBar))
        {
            scope_node = scope_node->get_Parent();
        }
    }
    else if (m_node->is_Gen(gen_tool) || m_node->is_Gen(gen_tool_dropdown))
    {
        scope_node = m_node->get_Parent();
        while (scope_node && !scope_node->is_Gen(gen_ToolBar) && !scope_node->is_Gen(gen_wxToolBar))
        {
            scope_node = scope_node->get_Parent();
        }
    }
    else if (m_node->is_Gen(gen_ribbonTool) || m_node->is_Gen(gen_ribbonButton) ||
             m_node->is_Gen(gen_ribbonGalleryItem))
    {
        scope_node = m_node->get_Parent();
        while (scope_node && !scope_node->is_Gen(gen_RibbonBar) &&
               !scope_node->is_Gen(gen_wxRibbonBar))
        {
            scope_node = scope_node->get_Parent();
        }
    }

    if (!scope_node)
    {
        return;
    }

    auto collect = [&](Node* child, auto&& collect) -> void
    {
        if (child != m_node && child->HasValue(prop_id))
        {
            const wxue::string identifier = NodeProperty::get_PropId(child->as_string(prop_id));
            if (!identifier.empty() && !identifier.starts_with("wx"))
            {
                m_form_ids.emplace(identifier.wx());
            }
        }

        for (const auto& iter: child->get_ChildNodePtrs())
        {
            collect(iter.get(), collect);
        }
    };

    collect(scope_node, collect);
}

void IDEditorDlg::UpdateDuplicateIdWarning()
{
    const bool was_shown = m_infoBar->IsShown();

    const wxue::string id_name = (!m_node || m_radioBtn_Standard->GetValue()) ?
                                     wxue::string {} :
                                     NodeProperty::get_PropId(BuildCompleteId().utf8_string());

    const bool is_duplicate =
        !id_name.empty() && !id_name.starts_with("wx") && m_form_ids.contains(id_name.wx());

    if (is_duplicate)
    {
        m_infoBar->ShowMessage(
            wxString::Format("The ID \"%s\" is already used by another control.", id_name.wx()),
            wxICON_WARNING);

        // The dialog is not resizeable, so the info bar would otherwise clip the OK/Cancel
        // buttons. Only resize when the bar transitions from hidden to shown; otherwise the
        // dialog would change size on every keystroke.
        if (!was_shown)
        {
            Layout();
            Fit();
        }
    }
    else if (was_shown)
    {
        m_infoBar->Dismiss();
        Layout();
        Fit();
    }
}

void IDEditorDlg::OnMakeUnique([[maybe_unused]] wxCommandEvent& event)
{
    const wxString original_id = m_textID->GetValue();
    wxString stem = original_id;

    const wxString digits("0123456789");
    while (!stem.empty() && digits.Contains(stem.Last()))
    {
        stem.RemoveLast();
    }

    if (stem.empty())
    {
        stem = original_id;
    }

    if (stem.empty())
    {
        return;
    }

    for (int i = 2;; ++i)
    {
        const wxString candidate = stem + wxString::Format("%d", i);
        if (!m_form_ids.contains(BuildIdName(candidate)))
        {
            m_textID->SetValue(candidate);
            break;
        }
    }

    wxCommandEvent dummy_event;
    OnComboSelect(dummy_event);
}

void IDEditorDlg::OnComboSelect([[maybe_unused]] wxCommandEvent& event)
{
    m_final_id->SetLabel(BuildCompleteId());
    UpdateDuplicateIdWarning();
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
