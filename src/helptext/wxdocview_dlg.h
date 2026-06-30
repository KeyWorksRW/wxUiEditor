/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog for displaying wxWidgets documentation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "wxdocview_base.h"

#include "data/include/utils.h"

class DocArchive;
class SearchPanel;

class wxDocView : public wxDocView_base
{
public:
    wxDocView();  // If you use this constructor, you must call Create(parent)
    wxDocView(wxWindow* parent);
    ~wxDocView() override;

    SearchPanel* get_SearchPanel() const noexcept { return m_search_panel.get(); }

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
    void OnInit(wxInitDialogEvent& event) override;

private:
    std::unique_ptr<SearchPanel> m_search_panel;

    bool m_svg_logo_registered = false;
};
