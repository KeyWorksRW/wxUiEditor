/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for inserting an item into a wxGridBagSizer node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/stockitem.h>

#include "gridbag_item.h"  // auto-generated: gridbag_item_base.h and gridbag_item_base.cpp

#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_gridbag.h"  // GridBag -- Create and modify a node containing a wxGridBagSizer

GridBagItem::GridBagItem(wxWindow* parent) : GridBagItemBase(parent) {}

void GridBagItem::OnInit(wxInitDialogEvent& WXUNUSED(event))
{
    if (auto cur_node = wxGetFrame().GetSelectedNode(); cur_node)
    {
        if (cur_node->isGen(gen_wxGridBagSizer))
        {
            m_gbsizer = cur_node;
            GridBag gb(cur_node);
            m_max_column = gb.GetMaxColumn();
            m_max_row = gb.GetMaxRow();
            m_spin_row->SetValue(m_max_row + 1);
        }
        else if (cur_node->GetParent()->isGen(gen_wxGridBagSizer))
        {
            m_gbsizer = cur_node->GetParent();
            GridBag gb(m_gbsizer);
            m_max_column = gb.GetMaxColumn();
            m_max_row = gb.GetMaxRow();

            m_spin_row->SetValue(cur_node->prop_as_int(prop_row));
            m_spin_column->SetValue(GetNewColumn(cur_node->prop_as_int(prop_row)));
        }
    }

    m_infoBar->AddButton(wxID_HIGHEST + 1, "Insert");
    m_infoBar->Bind(wxEVT_COMMAND_BUTTON_CLICKED, &GridBagItem::OnOK, this, wxID_HIGHEST + 1);
}

void GridBagItem::OnOK(wxCommandEvent& WXUNUSED(event))
{
    m_column = m_spin_column->GetValue();
    m_row = m_spin_row->GetValue();
    m_column_span = m_spin_span_column->GetValue();
    m_row_span = m_spin_span_row->GetValue();

    if (m_infoBar->IsShown())
    {
        m_action = m_radio_column->GetValue() ? action_insert_column : action_insert_row;
    }

    EndModal(wxID_OK);
}

void GridBagItem::OnColumn(wxSpinEvent& WXUNUSED(event))
{
    if (m_gbsizer)
    {
        auto row = m_spin_row->GetValue();
        auto max_column = GetNewColumn(row);
        auto column = m_spin_column->GetValue();
        if (column <= max_column)
        {
            bool is_inuse { false };
            for (size_t child_idx = 0; child_idx < m_gbsizer->GetChildCount(); ++child_idx)
            {
                if (m_gbsizer->GetChild(child_idx)->prop_as_int(prop_row) == row)
                {
                    if (column < m_gbsizer->GetChild(child_idx)->prop_as_int(prop_column))
                        continue;
                    else if (column > m_gbsizer->GetChild(child_idx)->prop_as_int(prop_column) +
                                          m_gbsizer->GetChild(child_idx)->prop_as_int(prop_colspan))
                        continue;
                    is_inuse = true;
                    break;
                }
            }

            if (is_inuse)
            {
                m_infoBar->ShowMessage("This column is already in use.", wxICON_INFORMATION);
                m_radio_column->Show();
                m_radio_column->SetValue(true);
                m_radio_row->Show();
                m_radio_row->SetValue(false);
                Fit();
            }
            else if (m_infoBar->IsShown())
            {
                m_radio_column->Hide();
                m_radio_row->Hide();
                m_infoBar->Dismiss();
                Fit();
            }
        }
        else
        {
            if (m_infoBar->IsShown())
            {
                m_radio_column->Hide();
                m_radio_row->Hide();
                m_infoBar->Dismiss();
                Fit();
            }
        }
    }
}

void GridBagItem::OnRow(wxSpinEvent& WXUNUSED(event))
{
    auto new_row = m_spin_row->GetValue();
    if (new_row > m_max_row)
    {
        m_spin_column->SetValue(0);
    }
    else if (m_gbsizer)
    {
        m_spin_column->SetValue(GetNewColumn(new_row) + 1);
    }

    if (m_infoBar->IsShown())
    {
        m_radio_column->Hide();
        m_radio_row->Hide();
        m_infoBar->Dismiss();
        Fit();
    }
}

int GridBagItem::GetNewColumn(int row)
{
    int new_column = 0;
    for (size_t child_idx = 0; child_idx < m_gbsizer->GetChildCount(); ++child_idx)
    {
        if (m_gbsizer->GetChild(child_idx)->prop_as_int(prop_row) == row)
        {
            auto column = m_gbsizer->GetChild(child_idx)->prop_as_int(prop_column) +
                          m_gbsizer->GetChild(child_idx)->prop_as_int(prop_colspan);
            if (column > new_column)
                new_column = column;
        }
    }
    return new_column;
}
