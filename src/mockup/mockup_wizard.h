//////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

#include <wx/bitmap.h>  // wxBitmap class interface
#include <wx/button.h>  // wxButtonBase class
#include <wx/sizer.h>   // provide wxSizer class for layout

class Node;
class wxStaticBitmap;
class wxWizardSizer;

class MockupWizardPage : public wxPanel
{
public:
    MockupWizardPage(Node* node, wxObject* parent);
    bool HasBitmap() { return m_bitmap.IsOk(); }
    wxBitmap GetBitmap() { return m_bitmap; }

private:
    wxBitmap m_bitmap { wxNullBitmap };
};

class MockupWizard : public wxPanel
{
public:
    MockupWizard(wxWindow* parent, Node* node);

    void AddPage(MockupWizardPage* page);
    size_t GetPageCount() { return m_pages.size(); }
    void SetSelection(size_t pageIndex);

protected:
    void OnBackOrNext(wxCommandEvent& event);
    void AddBitmapRow(wxBoxSizer* mainColumn);
    void AddButtonRow(wxBoxSizer* mainColumn);

private:
    wxBoxSizer* m_window_sizer;  // Contains both the bitmap and the wizard page
    wxBoxSizer* m_column_sizer;  // Contains the actual page to the right of any bitmap
    wxBoxSizer* m_sizerBmpAndPage;
    wxBoxSizer* m_sizerPage;

    wxButton* m_btnPrev;
    wxButton* m_btnNext;

    size_t m_cur_page_index { tt::npos };

    Node* m_wizard_node;

    wxSize m_top_min_size { 0, 0 };

    wxSize m_size_bmp { 0, 0 };
    wxBitmap m_bitmap { wxNullBitmap };
    wxStaticBitmap* m_statbmp { nullptr };

    std::vector<MockupWizardPage*> m_pages;
};
