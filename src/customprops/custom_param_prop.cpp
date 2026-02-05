/////////////////////////////////////////////////////////////////////////////
// Purpose:   Derived wxStringProperty class for custom control parameters
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "custom_param_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

EditParamProperty::EditParamProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

EditParamsDialog::EditParamsDialog(wxWindow* parent, NodeProperty* prop) :
    GridPropertyDlgBase(parent)
{
    m_prop = prop;
};

void EditParamsDialog::OnInit(wxInitDialogEvent& /* event unused */)
{
    m_prop_label->SetLabel("Custom Control Parameters");
    m_grid->SetColLabelValue(0, "Parameter");
    // m_grid->SetColFormatCustom(0, wxGRID_VALUE_CHOICE);

    auto fields = m_prop->as_ArrayString(',');

    if ((to_int) fields.size() > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(to_int(fields.size()) - m_grid->GetNumberRows());
    }

    // Unfortunately, wxGrid doesn't auto-size the column width correctly. Getting the text extent
    // of the longest line including an additional space at the end solves the problem, at least
    // running on Windows 11.
    auto col_width = m_grid->GetTextExtent("my_special_parameter_name_here ");
    m_grid->SetDefaultColSize(col_width.GetWidth(), true);

    // clang-format off
    const wxString choices[] = {
        "${parent}",
        "self",
        "this",
        "${id}",
        "${pos}",
        "${size}",
        "${window_style}",
        "${window_extra_style}",
        "${window_name}",
    };
    // clang-format on

    for (int row = 0; auto& iter: fields)
    {
        m_grid->SetCellEditor(row, 0, new wxGridCellChoiceEditor(WXSIZEOF(choices), choices, true));
        m_grid->SetCellValue(row, 0, iter);
        m_grid->SetRowLabelValue(row, wxue::itoa(row));
        ++row;
    }

    m_grid->DeleteCols(1, 1);  // Remove the second column

    Fit();
}

void EditParamsDialog::OnOK(wxCommandEvent& event)
{
    m_value.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        if (m_value.size())
        {
            m_value += ", ";
        }
        m_value += m_grid->GetCellValue(row, 0);
        m_value.Trim();
    }

    event.Skip();
}

void EditParamsDialog::OnUpdateUI(wxUpdateUIEvent& /* event unused */)
{
    auto array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, array.size() > 0);
    m_toolBar->EnableTool(id_UndoDeleteRow, m_deleted_col_0.size() > 0);
}

void EditParamsDialog::OnNewRow(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    auto new_row = m_grid->GetNumberRows() - 1;

    // clang-format off
    const wxString choices[] = {
        "${parent}",
        "self",
        "this",
        "${id}",
        "${pos}",
        "${size}",
        "${window_style}",
        "${window_extra_style}",
        "${window_name}",
    };
    // clang-format on
    m_grid->SetCellEditor(new_row, 0, new wxGridCellChoiceEditor(WXSIZEOF(choices), choices, true));
    m_grid->SetRowLabelValue(new_row, wxue::itoa(new_row));
    m_grid->SelectRow(new_row);
    m_grid->SetCellValue(new_row, 0, wxEmptyString);

    Fit();
}

void EditParamsDialog::OnDeleteRow(wxCommandEvent& /* event unused */)
{
    auto array = m_grid->GetSelectedRows();
    if (array.empty())
    {
        wxMessageBox("No rows selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    for (auto iter = array.rbegin(); iter != array.rend(); ++iter)
    {
        m_deleted_col_0 = m_grid->GetCellValue(*iter, 0);
        m_grid->DeleteRows(*iter);
    }
    Fit();
}

void EditParamsDialog::OnUndoDelete(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    if (m_deleted_col_0.size())
    {
        m_grid->SetCellValue(m_grid->GetNumberRows() - 1, 0, m_deleted_col_0);
        m_deleted_col_0.clear();
    }
    m_grid->SelectRow(m_grid->GetNumberRows() - 1);

    Fit();
}

bool EditParamsDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                           wxPGProperty* /* property unused */)
{
    EditParamsDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
