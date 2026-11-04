/////////////////////////////////////////////////////////////////////////////
// Purpose:   Property editor for wxBitmapComboBox contents
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <algorithm>  // std::max

#include <wx/filedlg.h>            // wxFileDialog
#include <wx/filename.h>           // wxFileName - encapsulates a file path
#include <wx/grid.h>               // wxGrid
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "bmp_combo_prop.h"

#include "../nodes/node_prop.h"  // NodeProperty class
#include "image_handler.h"       // ImageHandler class
#include "mainframe.h"           // MainFrame -- Main window frame
#include "project_handler.h"     // ProjectHandler class

#include "keywords.h"

#include "wxue_namespace/wxue_string.h"  // wxue::string, wxue::itoa

BmpComboProperty::BmpComboProperty(const wxString& label, NodeProperty* prop) :
    wxStringProperty(label, wxPG_LABEL, prop->as_wxString()),
    m_prop(prop)
{
}

BmpComboDialog::BmpComboDialog(wxWindow* parent, NodeProperty* prop) : GridPropertyDlgBase(parent)
{
    m_prop = prop;
    m_grid->Bind(wxEVT_GRID_CELL_LEFT_DCLICK, &BmpComboDialog::OnCellLeftDClick, this);
}

void BmpComboDialog::OnInit(wxInitDialogEvent& /* event unused */)
{
    m_prop_label->SetLabel("Bitmap Combo Contents");
    m_grid->SetColLabelValue(0, "Text");
    m_grid->SetColLabelValue(1, "Bitmap");

    m_items = m_prop->as_bmp_combo_items();
    if (m_items.empty())
    {
        m_items.emplace_back(NODEPROP_BMP_COMBO_ITEM {});
    }

    if (static_cast<int>(m_items.size()) > m_grid->GetNumberRows())
    {
        m_grid->AppendRows(static_cast<int>(m_items.size()) - m_grid->GetNumberRows());
    }

    // Unfortunately, wxGrid doesn't auto-size the column width correctly. Getting the text extent
    // of the longest line including an additional space at the end solves the problem, at least
    // running on Windows 11.
    int col_width = m_grid->GetTextExtent("Text ").GetWidth();
    for (const auto& item: m_items)
    {
        col_width = std::max(col_width, m_grid->GetTextExtent(item.label.wx() + ' ').GetWidth());
    }
    m_grid->SetColSize(0, col_width);

    for (int row = 0; const auto& item: m_items)
    {
        m_grid->SetCellValue(row, 0, item.label);
        m_grid->SetCellValue(row, 1, item.bitmap);
        m_grid->SetRowLabelValue(row, wxue::itoa(row));
        ++row;
    }

    m_help_text->SetLabel(
        "Double-click a Bitmap cell to choose an image file. SVG images default to 16x16.");

    m_help_text->Show();
    Fit();
}

void BmpComboDialog::OnCellLeftDClick(wxGridEvent& event)
{
    if (event.GetCol() != 1)
    {
        event.Skip();
        return;
    }

    wxFileDialog file_dlg(this, "Open Image", wxFileName(Project.get_ProjectPath().wx()).GetPath(),
                          wxEmptyString, "Image files|*.png;*.bmp;*.ico;*.svg;*.xpm|All files|*.*",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (file_dlg.ShowModal() == wxID_OK)
    {
        wxFileName file(file_dlg.GetPath());
        file.MakeRelativeTo(Project.get_ProjectPath().wx());
        wxString name = file.GetFullPath();
        wxue::back_slashesto_forward(name);
        m_grid->SetCellValue(event.GetRow(), 1, name);
    }

    event.Skip();
}

void BmpComboDialog::OnOK(wxCommandEvent& event)
{
    m_items.clear();
    for (int row = 0; row < m_grid->GetNumberRows(); ++row)
    {
        NODEPROP_BMP_COMBO_ITEM item;
        item.label.assign_wx(m_grid->GetCellValue(row, 0));
        item.bitmap.assign_wx(m_grid->GetCellValue(row, 1));
        m_items.push_back(item);
    }
    m_value = m_prop->convert_bmp_combo_items(m_items);

    // Register (or embed) every bitmap so that code generation can find it.
    for (const auto& item: m_items)
    {
        if (item.bitmap.empty())
        {
            continue;
        }

        // Bind through a base-class reference so the string concatenation below is unambiguous.
        const std::string& bitmap_path = item.bitmap;
        std::string description;
        if (bitmap_path.ends_with(".svg"))
        {
            description = "SVG;" + bitmap_path + ";[16,16]";
        }
        else
        {
            description = "Embed;" + bitmap_path;
        }

        wxue::StringVector parts(description, ';', wxue::TRIM::both);
        ProjectImages.UpdateBundle(&parts, m_prop->getNode());
    }

    event.Skip();
}

void BmpComboDialog::OnCancel(wxCommandEvent& event)
{
    // See comment in OnOK() of sb_fields_prop.cpp about why this is necessary.
    m_grid->GetEventHandler()->DeletePendingEvents();

    event.Skip();
}

void BmpComboDialog::OnUpdateUI(wxUpdateUIEvent& /* event unused */)
{
    const wxArrayInt array = m_grid->GetSelectedRows();
    m_toolBar->EnableTool(id_DeleteRow, !array.empty());
    m_toolBar->EnableTool(id_UndoDeleteRow, !m_deleted_col_0.empty());
}

void BmpComboDialog::OnNewRow(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    const int new_row = m_grid->GetNumberRows() - 1;
    m_grid->SetRowLabelValue(new_row, wxue::itoa(new_row));
    m_grid->SelectRow(new_row);
    m_grid->SetCellValue(new_row, 0, wxString {});
    m_grid->SetCellValue(new_row, 1, wxString {});
    Fit();
}

void BmpComboDialog::OnDeleteRow(wxCommandEvent& /* event unused */)
{
    wxArrayInt array = m_grid->GetSelectedRows();
    if (array.empty())
    {
        wxMessageBox("No rows selected", "Error", wxOK | wxICON_ERROR);
        return;
    }

    m_deleted_col_0 = m_grid->GetCellValue(array[0], 0);
    m_deleted_col_1 = m_grid->GetCellValue(array[0], 1);

    for (wxArrayInt::const_reverse_iterator iter = array.rbegin(); iter != array.rend(); ++iter)
    {
        m_grid->DeleteRows(*iter);
    }
    Fit();
}

void BmpComboDialog::OnUndoDelete(wxCommandEvent& /* event unused */)
{
    m_grid->AppendRows(1);
    if (!m_deleted_col_0.empty())
    {
        m_grid->SetCellValue(m_grid->GetNumberRows() - 1, 0, m_deleted_col_0);
        m_deleted_col_0.clear();
        m_grid->SetCellValue(m_grid->GetNumberRows() - 1, 1, m_deleted_col_1);
        m_deleted_col_1.clear();
    }
    m_grid->SetRowLabelValue(m_grid->GetNumberRows() - 1, wxue::itoa(m_grid->GetNumberRows() - 1));
    m_grid->SelectRow(m_grid->GetNumberRows() - 1);
    Fit();
}

bool BmpComboDialogAdapter::DoShowDialog(wxPropertyGrid* /* propGrid unused */,
                                         wxPGProperty* /* property unused */)
{
    BmpComboDialog dlg(wxGetFrame().getWindow(), m_prop);
    if (dlg.ShowModal() == wxID_OK)
    {
        SetValue(dlg.GetResults());
        return true;
    }

    return false;
}
