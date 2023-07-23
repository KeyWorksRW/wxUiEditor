/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRearangeList contents editor
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "rearrange_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;

RearrangeProperty::RearrangeProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

RearrangeDialog::RearrangeDialog(wxWindow* parent, NodeProperty* prop) : GridPropertyDlg(parent)
{
    m_prop = prop;
};

void RearrangeDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_prop_label->SetLabel("Listbox Contents");
    m_grid->SetColLabelValue(0, "Checked");
    m_grid->SetColLabelValue(1, "Label");
    m_grid->SetColFormatCustom(0, wxGRID_VALUE_BOOL);
    m_grid->SetColFormatCustom(1, wxGRID_VALUE_STRING);

    auto contents = m_prop->as_checklist_items();
    if ((to_int) contents.size() > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(to_int(contents.size()) - m_grid->GetNumberRows());
    }

    m_label_width = m_grid->GetTextExtent("This is a sample label used for width.").GetWidth();

    for (int row = 0; auto& iter: contents)
    {
        m_grid->SetCellValue(row, 0, iter.checked);
        m_grid->SetCellValue(row, 1, iter.label);
        int width = m_grid->GetTextExtent(iter.label).GetWidth();
        if (width > m_label_width)
            m_label_width = width;
        m_grid->SetRowLabelValue(row, " ");
        ++row;
    }

    m_grid->SetColSize(1, m_label_width);

    m_help_text->SetLabel("Labels cannot contain a pipe (|) or semi-colon (;) character.");
    m_help_text->Show();

    Fit();
}

void RearrangeDialog::OnOK(wxCommandEvent& event)
{
    m_contents.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        NODEPROP_CHECKLIST_ITEM item;
        item.checked.assign_wx(m_grid->GetCellValue(row, 0));
        item.label.assign_wx(m_grid->GetCellValue(row, 1));
        m_contents.push_back(item);
    }
    m_value = m_prop->convert_checklist_items(m_contents);

    // REVIEW: [Randalphwa - 09-01-2022] This shouldn't be necessary, but in debug builds, we sometimes get
    // a warning about undeleted events. Since none of the other custom property editors have this issue, it's most
    // likely due to something in m_grid.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void RearrangeDialog::OnCancel(wxCommandEvent& event)
{
    // See comment in OnOK() about why this is necessary.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void RearrangeDialog::OnUpdateUI(wxUpdateUIEvent& WXUNUSED(event))
{
    auto array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, array.size() > 0);
    m_toolBar->EnableTool(id_UndoDeleteRow, m_deleted_col_0.size());
}

void RearrangeDialog::OnNewRow(wxCommandEvent& WXUNUSED(event))
{
    m_grid->AppendRows(1);
    auto new_row = m_grid->GetNumberRows() - 1;
    m_grid->SetRowLabelValue(new_row, " ");
    m_grid->SelectRow(new_row);
    Fit();
}

void RearrangeDialog::OnDeleteRow(wxCommandEvent& WXUNUSED(event))
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
        m_deleted_col_1 = m_grid->GetCellValue(*iter, 1);
        m_grid->DeleteRows(*iter);
    }
    Fit();
}

void RearrangeDialog::OnUndoDelete(wxCommandEvent& WXUNUSED(event))
{
    m_grid->AppendRows(1);
    if (m_deleted_col_0.size())
    {
        m_grid->SetCellValue(m_grid->GetNumberRows() - 1, 0, m_deleted_col_0);
        m_deleted_col_0.clear();
        m_grid->SetCellValue(m_grid->GetNumberRows() - 1, 1, m_deleted_col_1);
        m_deleted_col_1.clear();
    }
    m_grid->SelectRow(m_grid->GetNumberRows() - 1);
    Fit();
}

bool RearrangeDialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    RearrangeDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
