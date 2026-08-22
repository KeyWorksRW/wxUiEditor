/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to lookup and insert a widget
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [08-22-2026]

#pragma once

#include "insert_widget_base.h"  // InsertWidgetBase

#include "wxue_namespace/wxue_string.h"  // wxue::string

class InsertWidget : public InsertWidgetBase
{
public:
    InsertWidget() {}  // If you use this constructor, you must call Create(parent)
    InsertWidget(wxWindow* parent);

    wxue::string GetWidget() { return m_widget; }

protected:
    // Handlers for InsertWidgetBase events

    void OnInit(wxInitDialogEvent& event) override;
    void OnChangeLimit(wxCommandEvent& event) override;
    void OnNameText(wxCommandEvent& event) override;
    void OnListBoxDblClick(wxCommandEvent& event) override;
    void OnOK(wxCommandEvent& event) override;
    void OnKeyDown(wxKeyEvent& event) override;

private:
    void RebuildList();
    wxue::string m_widget;
};
