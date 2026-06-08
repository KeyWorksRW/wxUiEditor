/////////////////////////////////////////////////////////////////////////////
// Purpose:
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "wxdocview_base.h"

class wxDocView : public wxDocView_base
{
public:
    wxDocView();  // If you use this constructor, you must call Create(parent)
    wxDocView(wxWindow* parent);

protected:
    // Handlers for wxDocView_base events
    void OnPageChanged(wxBookCtrlEvent& event) override;
    void OnSearchCancel(wxCommandEvent& event) override;
    void OnSearchTextChanged(wxCommandEvent& event) override;
    void OnDisplaySearchListItem(wxCommandEvent& event) override;
    void OnTextKeyDown(wxKeyEvent& event) override;
    void OnIndexTextEnter(wxCommandEvent& event) override;
    void OnIndexTextChange(wxCommandEvent& event) override;
    void OnDblClickListBox(wxCommandEvent& event) override;
    void OnHome(wxCommandEvent& event) override;
    void OnHtmlLink(wxHtmlLinkEvent& event) override;
};
