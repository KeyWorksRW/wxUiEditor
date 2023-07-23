/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for status bar fields
// Author:    Ralph Walden
// Copyright: Copyright (c) 2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "sb_fields_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "mainframe.h"           // MainFrame -- Main window frame

// Defined in base_panel.cpp
extern const char* g_u8_cpp_keywords;

SBarFieldsProperty::SBarFieldsProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()), m_prop(prop)
{
}

SBarFieldsDialog::SBarFieldsDialog(wxWindow* parent, NodeProperty* prop) : GridPropertyDlg(parent)
{
    m_prop = prop;
};

void SBarFieldsDialog::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    m_prop_label->SetLabel("StatusBar Fields");
    m_grid->SetColLabelValue(0, "Style");
    m_grid->SetColLabelValue(1, "Width");
    m_grid->SetColFormatCustom(0, wxGRID_VALUE_CHOICE);
    m_grid->SetColFormatCustom(1, wxGRID_VALUE_NUMBER);

    auto fields = m_prop->as_statusbar_fields();
    if (tt::is_digit(m_prop->as_string()[0]))
    {
        fields.clear();
        auto total_fields = m_prop->as_int();
        for (int idx = 0; idx < total_fields; ++idx)
        {
            NODEPROP_STATUSBAR_FIELD field { "wxSB_NORMAL", "-1" };
            fields.push_back(field);
        }
    }

    if (fields.empty())
    {
        NODEPROP_STATUSBAR_FIELD field { "wxSB_NORMAL", "-1" };
        fields.push_back(field);
    }

    if ((to_int) fields.size() > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(to_int(fields.size()) - m_grid->GetNumberRows());
    }

    // Unfortunately, wxGrid doesn't auto-size the column width correctly. Getting the text extent of the longest line
    // including an additional space at the end solves the problem, at least running on Windows 11.
    auto col_width = m_grid->GetTextExtent("wxSB_NORMAL ");
    m_grid->SetDefaultColSize(col_width.GetWidth(), true);

    for (int row = 0; auto& iter: fields)
    {
        if (wxGridCellChoiceEditor* editor = static_cast<wxGridCellChoiceEditor*>(m_grid->GetCellEditor(row, 0)); editor)
            editor->SetParameters(wxT("wxSB_NORMAL,wxSB_FLAT,wxSB_RAISED,wxSB_SUNKEN"));
        m_grid->SetCellValue(row, 0, iter.style);
        m_grid->SetCellValue(row, 1, iter.width);
        m_grid->SetRowLabelValue(row, " ");
        ++row;
    }

    m_help_text->SetLabel(
        "A positive width indicates a fixed width field, a negative width indicates a proportional field.");

    // Force the width to get wrap in a way that makes the text the most clear.
    auto width = m_help_text->GetTextExtent("A positive width indicates a fixed width field, ").GetWidth();
    m_help_text->Wrap(width);

    m_help_text->Show();
    Fit();
}

void SBarFieldsDialog::OnOK(wxCommandEvent& event)
{
    m_fields.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        NODEPROP_STATUSBAR_FIELD field;
        field.style.assign_wx(m_grid->GetCellValue(row, 0));
        field.width.assign_wx(m_grid->GetCellValue(row, 1));
        m_fields.push_back(field);
    }
    m_value = m_prop->convert_statusbar_fields(m_fields);
    if (m_value == "wxSB_NORMAL|-1")
        m_value = "1";

    // REVIEW: [Randalphwa - 09-01-2022] This shouldn't be necessary, but in debug builds, we sometimes get
    // a warning about undeleted events. Since none of the other custom property editors have this issue, it's most
    // likely due to something in m_grid.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void SBarFieldsDialog::OnCancel(wxCommandEvent& event)
{
    // See comment in OnOK() about why this is necessary.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void SBarFieldsDialog::OnUpdateUI(wxUpdateUIEvent& WXUNUSED(event))
{
    auto array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, array.size() > 0);
    m_toolBar->EnableTool(id_UndoDeleteRow, m_deleted_col_0.size());
}

void SBarFieldsDialog::OnNewRow(wxCommandEvent& WXUNUSED(event))
{
    m_grid->AppendRows(1);
    auto new_row = m_grid->GetNumberRows() - 1;
    m_grid->SetRowLabelValue(new_row, " ");
    m_grid->SelectRow(new_row);
    m_grid->SetCellValue(new_row, 0, "wxSB_NORMAL");
    m_grid->SetCellValue(new_row, 1, "-1");
    Fit();
}

void SBarFieldsDialog::OnDeleteRow(wxCommandEvent& WXUNUSED(event))
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

void SBarFieldsDialog::OnUndoDelete(wxCommandEvent& WXUNUSED(event))
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

bool SBarFieldsDialogAdapter::DoShowDialog(wxPropertyGrid* WXUNUSED(propGrid), wxPGProperty* WXUNUSED(property))
{
    SBarFieldsDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
