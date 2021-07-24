// Purpose:   Dialog to lookup and insert a widget
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "insertwidget_base.h"

class InsertWidget : public InsertWidgetBase
{
public:
    InsertWidget(wxWindow* parent = nullptr);
    ttlib::cstr GetWidget() { return m_widget; }

protected:
    // Handlers for InsertWidgetBase events
    void OnInit(wxInitDialogEvent& WXUNUSED(event)) override;
    void OnNameText(wxCommandEvent& WXUNUSED(event)) override;
    void OnListBoxDblClick(wxCommandEvent& WXUNUSED(event)) override;
    void OnOK(wxCommandEvent& WXUNUSED(event)) override;

private:
    ttlib::cstr m_widget;
};
