/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for Growable Sizer Columns
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "sizer_grow_columns.h"

#include "../nodes/node_prop.h"                 // NodeProperty class
#include "mainframe.h"                          // MainFrame -- Main window frame
#include "wxue_namespace/wxue_string_vector.h"  // wxue::StringVector, wxue::is_digit, wxue::atoi

GrowColumnsProperty::GrowColumnsProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

GrowColumnsDialog::GrowColumnsDialog(wxWindow* parent, NodeProperty* prop) :
    GridPropertyDlgBase(parent)
{
    m_prop = prop;
};

void GrowColumnsDialog::OnInit([[maybe_unused]] wxInitDialogEvent& event)
{
    m_prop_label->SetLabel("Growable Columns");
    m_grid->SetColLabelValue(0, "Column");
    m_grid->SetColLabelValue(1, "Proportion");
    m_grid->SetColFormatCustom(0, wxGRID_VALUE_NUMBER);
    m_grid->SetColFormatCustom(1, wxGRID_VALUE_NUMBER);

    std::vector<GrowColumnsEntry> entries;
    wxue::StringVector fields(m_prop->as_string(), ",", wxue::TRIM::both);
    for (auto& iter: fields)
    {
        if (wxue::is_digit(iter[0]))
        {
            GrowColumnsEntry entry;
            entry.column = wxue::atoi(iter);
            if (auto pos = iter.find(':'); pos != std::string::npos)
            {
                entry.proportion = wxue::atoi(iter.substr(pos + 1));
            }
            else
            {
                entry.proportion = -1;
            }
            entries.push_back(entry);
        }
    }

    if ((to_int) entries.size() > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(to_int(entries.size()) - m_grid->GetNumberRows());
    }

    // Unfortunately, wxGrid doesn't auto-size the column width correctly. Getting the text extent
    // of the longest line including an additional space at the end solves the problem, at least
    // running on Windows 11.
    const wxSize text_width = m_grid->GetTextExtent(" Proportion ");
    m_grid->SetDefaultColSize(text_width.GetWidth(), true);
    for (int row = 0; auto& iter: entries)
    {
        m_grid->SetCellEditor(row, 0, new wxGridCellNumberEditor(0, 99));
        m_grid->SetCellValue(row, 0, std::to_string(iter.column));
        m_grid->SetCellEditor(row, 1, new wxGridCellNumberEditor(-1, 100));
        m_grid->SetCellValue(row, 1, std::to_string(iter.proportion));
        m_grid->SetRowLabelValue(row, " ");
        ++row;
    }
    m_grow_columns = entries;

    m_help_text->SetLabel(
        "Proportion has the same meaning as the stretch factor for sizers (see wxBoxSizer) "
        "except that if all proportions are 0, then all columns are resized equally "
        "(instead of not being resized at all).");

    // Force the width to get wrap in a way that makes the text the most clear.
    const int width =
        m_help_text
            ->GetTextExtent(
                "Proportion has the same meaning as the stretch factor for sizers (see wxBoxSizer)")
            .GetWidth();
    m_help_text->Wrap(width);
    m_help_text->Show();

    m_toolBar->DeleteTool(id_UndoDeleteRow);  // Remove the Undo button

    Fit();
}

void GrowColumnsDialog::OnOK(wxCommandEvent& event)
{
    m_grow_columns.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        GrowColumnsEntry grow_entry;
        grow_entry.column = wxue::atoi(m_grid->GetCellValue(row, 0).ToStdString());
        grow_entry.proportion = wxue::atoi(m_grid->GetCellValue(row, 1).ToStdString());
        m_grow_columns.push_back(grow_entry);
    }

    m_value.clear();
    for (auto& iter: m_grow_columns)
    {
        if (iter.column >= 0)
        {
            if (!m_value.empty())
            {
                m_value += ",";
            }
            m_value += std::to_string(iter.column);
            if (iter.proportion != -1)
            {
                m_value += ":";
                m_value += std::to_string(iter.proportion);
            }
        }
    }

    event.Skip();
}

void GrowColumnsDialog::OnUpdateUI([[maybe_unused]] wxUpdateUIEvent& event)
{
    const wxArrayInt array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, !array.empty());
}

void GrowColumnsDialog::OnNewRow([[maybe_unused]] wxCommandEvent& event)
{
    m_grid->AppendRows(1);
    const int new_row = m_grid->GetNumberRows() - 1;

    int new_column = 0;
    for (auto& iter: m_grow_columns)
    {
        if (iter.column >= new_column)
        {
            new_column = iter.column + 1;
        }
    }

    m_grid->SetCellEditor(new_row, 0, new wxGridCellNumberEditor(0, 99));
    m_grid->SetCellValue(new_row, 0, std::to_string(new_column));
    m_grid->SetCellEditor(new_row, 1, new wxGridCellNumberEditor(-1, 100));
    m_grid->SetCellValue(new_row, 1, std::to_string(-1));
    m_grid->SetRowLabelValue(new_row, " ");
    Fit();
}

void GrowColumnsDialog::OnDeleteRow([[maybe_unused]] wxCommandEvent& event)
{
    wxArrayInt array = m_grid->GetSelectedRows();
    if (array.empty())
    {
        wxMessageBox("No rows selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    for (wxArrayInt::const_reverse_iterator iter = array.rbegin(); iter != array.rend(); ++iter)
    {
        m_grid->DeleteRows(*iter);
    }

    Fit();
}

[[nodiscard]] bool GrowColumnsDialogAdapter::DoShowDialog([[maybe_unused]] wxPropertyGrid* propGrid,
                                                          [[maybe_unused]] wxPGProperty* property)
{
    GrowColumnsDialog grow_dlg(wxGetFrame().getWindow(), m_prop);
    if (grow_dlg.ShowModal() == wxID_OK)
    {
        SetValue(grow_dlg.GetResults());
        return true;
    }

    return false;
}
