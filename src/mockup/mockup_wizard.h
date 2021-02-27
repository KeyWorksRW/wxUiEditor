//////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

#include <wx/button.h>  // wxButtonBase class
#include <wx/event.h>   // Event classes
#include <wx/sizer.h>   // provide wxSizer class for layout

class Node;

class MockupWizard : public wxPanel
{
public:
    MockupWizard(wxWindow* parent, Node* node);

    void AddPage(wxPanel* page);
    size_t GetPageCount() { return m_pages.size(); }
    void SetSelection(size_t pageIndex);

protected:
    void OnBackOrNext(wxCommandEvent& event);

private:
    wxBoxSizer* m_top_sizer;       // Contains both the bitmap and the wizard page
    wxBoxSizer* m_sizer_wiz_page;  // Contains the actual page to the right of any bitmap

    wxButton* m_btn_back;
    wxButton* m_btn_forward;

    size_t m_cur_page_index { tt::npos };

    Node* m_wizard_node;

    wxSize m_top_min_size { 0, 0 };
    wxSize m_size_bmp { 0, 0 };

    std::vector<wxPanel*> m_pages;
};
