/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for inserting an item into a wxGridBagSizer node
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "gridbag_item_base.h"

class Node;

class GridBagItem : public GridBagItemBase
{
public:
    GridBagItem(wxWindow* parent);

    enum
    {
        action_append,  // append column if row exists, else append row
        action_insert_column,
        action_insert_row,
    };

    auto GetColumn() const { return m_column; }
    auto GetRow() const { return m_row; }
    auto GetColumnSpan() const { return m_column_span; }
    auto GetRowSpan() const { return m_row_span; }

    auto GetAction() const { return m_action; }

protected:
    int GetNewColumn(int row);
    void OnColumn(wxSpinEvent& event) override;
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& WXUNUSED(event)) override;
    void OnRow(wxSpinEvent& event) override;

private:
    Node* m_gbsizer { nullptr };

    int m_column;
    int m_row;
    int m_column_span;
    int m_row_span;

    int m_max_column { -1 };
    int m_max_row { -1 };

    int m_action { action_append };
};
