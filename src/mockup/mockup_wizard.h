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
    wxBitmap* GetBitmapPtr() { return &m_bitmap; }

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
    wxSize& GetLargetPageSize() { return m_largest_page; }

    // MockupContent needs to call this after all children have been created in order to calculate the largest size needed to
    // display all children, and to resize any bitmap if bmp_placement has been set.
    void AllChildrenAdded();

protected:
    void OnBackOrNext(wxCommandEvent& event);
    void AddBitmapRow();
    void AddButtonRow();

    bool ResizeBitmap(wxBitmap& bmp);

private:
    wxBoxSizer* m_window_sizer;  // Top level sizer for entire window
    wxBoxSizer* m_column_sizer;  // Contains bitmap row, static line, and buttons
    wxBoxSizer* m_sizerBmpAndPage;
    wxBoxSizer* m_sizerPage { nullptr };

    wxButton* m_btnPrev;
    wxButton* m_btnNext;

    size_t m_cur_page_index { tt::npos };

    Node* m_wizard_node;

    wxSize m_largest_page { 0, 0 };
    wxSize m_largest_nonbmp_page { 0, 0 };

    wxSize m_size_bmp { 0, 0 };
    wxBitmap m_bitmap { wxNullBitmap };
    wxStaticBitmap* m_statbmp { nullptr };

    std::vector<MockupWizardPage*> m_pages;

    int m_bmp_placement;
    int m_border { 5 };
};
