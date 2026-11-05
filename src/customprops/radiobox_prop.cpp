/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for wxRadioBox items
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // std::max

#include <wx/grid.h>               // wxGrid
#include <wx/msgdlg.h>             // wxMessageBox
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "radiobox_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::itoa

RadioBoxProperty::RadioBoxProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

RadioBoxDlg::RadioBoxDlg(wxWindow* parent, NodeProperty* prop) : RadioBoxDlgBase(parent)
{
    m_prop = prop;
}

void RadioBoxDlg::OnInit(wxInitDialogEvent& /* event unused */)
{
    m_prop_label->SetLabel("Radio Box Items");
    m_grid->SetColLabelValue(0, "Text");
    m_grid->SetColLabelValue(1, "Enabled");
    m_grid->SetColLabelValue(2, "Show");
    m_grid->SetColLabelValue(3, "Tooltip");
    m_grid->SetColLabelValue(4, "Help");
    m_grid->SetColFormatBool(1);
    m_grid->SetColFormatBool(2);

    m_items = m_prop->as_radiobox_items();
    if (m_items.empty())
    {
        m_items.emplace_back(NODEPROP_RADIOBOX_ITEM {});
    }

    // A missing or empty enabled/show field means the item is enabled and shown.
    for (auto& item: m_items)
    {
        if (item.enabled.empty())
        {
            item.enabled = "1";
        }
        if (item.show.empty())
        {
            item.show = "1";
        }
    }

    if (static_cast<int>(m_items.size()) > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(static_cast<int>(m_items.size()) - m_grid->GetNumberRows());
    }

    // Unfortunately, wxGrid doesn't auto-size the column width correctly. Getting the text extent
    // of the longest line including an additional space at the end solves the problem, at least
    // running on Windows 11.
    int label_width = m_grid->GetTextExtent("Text ").GetWidth();
    int tooltip_width = m_grid->GetTextExtent("Tooltip ").GetWidth();
    int helptext_width = m_grid->GetTextExtent("Help ").GetWidth();
    for (const auto& item: m_items)
    {
        label_width =
            std::max(label_width, m_grid->GetTextExtent(item.label.wx() + ' ').GetWidth());
        tooltip_width =
            std::max(tooltip_width, m_grid->GetTextExtent(item.tooltip.wx() + ' ').GetWidth());
        helptext_width =
            std::max(helptext_width, m_grid->GetTextExtent(item.helptext.wx() + ' ').GetWidth());
    }
    m_grid->SetColSize(0, label_width);
    m_grid->SetColSize(3, tooltip_width);
    m_grid->SetColSize(4, helptext_width);

    // The Enabled and Show columns only display a checkbox, so keep them narrow.
    const int bool_width = m_grid->GetTextExtent("Enabled ").GetWidth();
    m_grid->SetColSize(1, bool_width);
    m_grid->SetColSize(2, bool_width);

    for (int row = 0; const auto& item: m_items)
    {
        m_grid->SetCellValue(row, 0, item.label);
        m_grid->SetCellValue(row, 1, item.enabled);
        m_grid->SetCellValue(row, 2, item.show);
        m_grid->SetCellValue(row, 3, item.tooltip);
        m_grid->SetCellValue(row, 4, item.helptext);
        m_grid->SetRowLabelValue(row, wxue::itoa(row));
        ++row;
    }

    m_help_text->SetLabel("Uncheck Enabled or Show to disable or hide an item. Tooltip and Help "
                          "text are optional.");

    m_help_text->Show();
    Fit();
}

void RadioBoxDlg::OnOK(wxCommandEvent& event)
{
    m_items.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        NODEPROP_RADIOBOX_ITEM item;
        item.label.assign_wx(m_grid->GetCellValue(row, 0));
        item.enabled.assign_wx(m_grid->GetCellValue(row, 1));
        if (item.enabled.empty())
        {
            item.enabled = "1";
        }
        item.show.assign_wx(m_grid->GetCellValue(row, 2));
        if (item.show.empty())
        {
            item.show = "1";
        }
        item.tooltip.assign_wx(m_grid->GetCellValue(row, 3));
        item.helptext.assign_wx(m_grid->GetCellValue(row, 4));
        m_items.push_back(item);
    }
    m_value = m_prop->convert_radiobox_items(m_items).wx();

    event.Skip();
}

void RadioBoxDlg::OnCancel(wxCommandEvent& event)
{
    // See comment in OnOK() of sb_fields_prop.cpp about why this is necessary.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void RadioBoxDlg::OnUpdateUI(wxUpdateUIEvent& /* event unused */)
{
    const wxArrayInt array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, !array.empty());
    m_toolBar->EnableTool(id_UndoDeleteRow, !m_deleted_label.empty());
}

void RadioBoxDlg::OnNewRow(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    const int new_row = m_grid->GetNumberRows() - 1;
    m_grid->SetRowLabelValue(new_row, wxue::itoa(new_row));
    m_grid->SetCellValue(new_row, 0, wxString {});
    m_grid->SetCellValue(new_row, 1, "1");
    m_grid->SetCellValue(new_row, 2, "1");
    m_grid->SetCellValue(new_row, 3, wxString {});
    m_grid->SetCellValue(new_row, 4, wxString {});
    m_grid->SelectRow(new_row);
    Fit();
}

void RadioBoxDlg::OnDeleteRow(wxCommandEvent& /* event unused */)
{
    const wxArrayInt array = m_grid->GetSelectedRows();
    if (array.empty())
    {
        wxMessageBox("No rows selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    m_deleted_label = m_grid->GetCellValue(array[0], 0);
    m_deleted_enabled = m_grid->GetCellValue(array[0], 1);
    m_deleted_show = m_grid->GetCellValue(array[0], 2);
    m_deleted_tooltip = m_grid->GetCellValue(array[0], 3);
    m_deleted_helptext = m_grid->GetCellValue(array[0], 4);

    for (wxArrayInt::const_reverse_iterator iter = array.rbegin(); iter != array.rend(); ++iter)
    {
        m_grid->DeleteRows(*iter);
    }
    Fit();
}

void RadioBoxDlg::OnUndoDelete(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    const int new_row = m_grid->GetNumberRows() - 1;
    if (!m_deleted_label.empty())
    {
        m_grid->SetCellValue(new_row, 0, m_deleted_label);
        m_deleted_label.clear();
    }
    if (!m_deleted_enabled.empty())
    {
        m_grid->SetCellValue(new_row, 1, m_deleted_enabled);
        m_deleted_enabled.clear();
    }
    if (!m_deleted_show.empty())
    {
        m_grid->SetCellValue(new_row, 2, m_deleted_show);
        m_deleted_show.clear();
    }
    if (!m_deleted_tooltip.empty())
    {
        m_grid->SetCellValue(new_row, 3, m_deleted_tooltip);
        m_deleted_tooltip.clear();
    }
    if (!m_deleted_helptext.empty())
    {
        m_grid->SetCellValue(new_row, 4, m_deleted_helptext);
        m_deleted_helptext.clear();
    }
    m_grid->SetRowLabelValue(new_row, wxue::itoa(new_row));
    m_grid->SelectRow(new_row);
    Fit();
}

bool RadioBoxDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                         wxPGProperty* /* property unused */)
{
    RadioBoxDlg dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
