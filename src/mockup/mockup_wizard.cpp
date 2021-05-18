/////////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard, used for Mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// A wxWizard derives from wxDialog which makes it unusable as a child of the wxPanel used by our Mockup panel. We emulate
// the functionality here, use similar methods to what that the real wxWizard uses (see wxWidgets/src/generic/wizard.cpp).

#include "pch.h"

#include <wx/dcmemory.h>  // wxMemoryDC base header
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/wizard.h>    // wxWizard class: a GUI control presenting the user with a

#include "mockup_wizard.h"

#include "auto_freeze.h"  // AutoFreeze -- Automatically Freeze/Thaw a window
#include "mainframe.h"    // App -- App class
#include "node.h"         // Node class

MockupWizard::MockupWizard(wxWindow* parent, Node* node) : wxPanel(parent)
{
    m_wizard_node = node;
    m_border = node->prop_as_int(prop_border);

    m_window_sizer = new wxBoxSizer(wxVERTICAL);
    m_column_sizer = new wxBoxSizer(wxVERTICAL);
    m_window_sizer->Add(m_column_sizer, wxSizerFlags(1).Expand());

    AddBitmapRow();

    m_column_sizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border());
    m_column_sizer->Add(0, 5, 0, wxEXPAND);

    AddButtonRow();

    SetSizer(m_window_sizer);

    m_btnPrev->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
    m_btnNext->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
}

void MockupWizard::AddBitmapRow()
{
    m_sizerBmpAndPage = new wxBoxSizer(wxHORIZONTAL);
    m_column_sizer->Add(m_sizerBmpAndPage, wxSizerFlags(1).Expand());
    m_column_sizer->Add(0, 5, 0, wxEXPAND);

    if (m_wizard_node->HasValue(prop_bitmap))
    {
        m_bitmap = m_wizard_node->prop_as_wxBitmap(prop_bitmap);
        if (m_bitmap.IsOk())
        {
            wxSize bitmapSize(wxDefaultSize);
            if (m_wizard_node->prop_as_int(prop_bmp_placement) > 0 && m_wizard_node->prop_as_int(prop_bmp_min_width) > 0)
            {
                ResizeBitmap(m_bitmap);
                bitmapSize.x = m_wizard_node->prop_as_int(prop_bmp_min_width);
            }
            m_statbmp = new wxStaticBitmap(this, wxID_ANY, m_bitmap, wxDefaultPosition, bitmapSize);
            m_sizerBmpAndPage->Add(m_statbmp, wxSizerFlags());
            m_sizerBmpAndPage->Add(5, 0, 0, wxEXPAND);

            m_size_bmp = m_bitmap.GetSize();
            m_size_bmp.IncBy(5);  // add border size
        }
    }

    m_sizerPage = new wxBoxSizer(wxHORIZONTAL);
    m_sizerBmpAndPage->Add(m_sizerPage, wxSizerFlags());

    // REVIEW: [KeyWorks - 05-18-2021] This should work, but unfortunately it can truncate one or more pages. Worse, it
    // causes the button row to sometimes shift locations when switching between pages.

    // m_sizerBmpAndPage->Add(m_sizerPage, wxSizerFlags(1).Border(wxALL, m_border));
}

void MockupWizard::AddButtonRow()
{
    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);

    m_column_sizer->Add(buttonRow, 0, wxALIGN_RIGHT);

    if (m_wizard_node->prop_as_string(prop_extra_style).contains("wxWIZARD_EX_HELPBUTTON"))
    {
        buttonRow->Add(new wxButton(this, wxID_HELP, "&Help"), 0, wxALL, 5);
#ifdef __WXMAC__
        // Put stretchable space between help button and others
        buttonRow->Add(0, 0, 1, wxALIGN_CENTRE, 0);
#endif
    }

    m_btnPrev = new wxButton(this, wxID_BACKWARD, "< &Back");
    m_btnNext = new wxButton(this, wxID_FORWARD, "&Next >");

    wxBoxSizer* backNextPair = new wxBoxSizer(wxHORIZONTAL);
    buttonRow->Add(backNextPair, 0, wxALL, 5);
    backNextPair->Add(m_btnPrev);
    backNextPair->Add(10, 0, 0, wxEXPAND);
    backNextPair->Add(m_btnNext);

    buttonRow->Add(new wxButton(this, wxID_CANCEL, "&Cancel"), 0, wxALL, 5);
}

void MockupWizard::SetSelection(size_t pageIndex)
{
    AutoFreeze(this);

    auto old_pageIndex = m_cur_page_index;

    if (pageIndex < m_pages.size())
    {
        bool hasPrev = pageIndex > 0;
        bool hasNext = pageIndex < m_pages.size() - 1;

        m_cur_page_index = pageIndex;

        m_btnPrev->Enable(hasPrev);

        wxString label = hasNext ? "&Next >" : "&Finish";
        if (label != m_btnNext->GetLabel())
        {
            m_btnNext->SetLabel(label);
        }

        m_btnNext->SetDefault();
    }

    if (old_pageIndex != pageIndex && pageIndex < m_pages.size())
    {
        wxBitmap bmpPrev = m_pages[old_pageIndex]->GetBitmap();

        auto bmp = m_pages[pageIndex]->GetBitmap();
        if (!bmp.IsOk())
            bmp = m_bitmap;
        if (!bmpPrev.IsOk())
            bmpPrev = m_bitmap;

        if (!bmp.IsSameAs(bmpPrev))
            m_statbmp->SetBitmap(bmp);

        m_pages[old_pageIndex]->Hide();
        m_pages[pageIndex]->Show();
        m_cur_page_index = pageIndex;
    }

    Fit();
    Layout();
}

void MockupWizard::OnBackOrNext(wxCommandEvent& event)
{
    if (event.GetId() == wxID_FORWARD && m_cur_page_index + 1 >= m_pages.size())
        return;
    else if (event.GetId() == wxID_BACKWARD && m_cur_page_index == 0)
        return;

    AutoFreeze(this);

    wxBitmap bmpPrev = m_pages[m_cur_page_index]->GetBitmap();

    m_pages[m_cur_page_index]->Hide();
    if (event.GetEventObject() == m_btnNext)
        m_cur_page_index++;
    else
        m_cur_page_index--;

    auto bmp = m_pages[m_cur_page_index]->GetBitmap();
    if (!bmp.IsOk())
        bmp = m_bitmap;
    if (!bmpPrev.IsOk())
        bmpPrev = m_bitmap;

    if (!bmp.IsSameAs(bmpPrev))
        m_statbmp->SetBitmap(bmp);

    m_pages[m_cur_page_index]->Show();

    SetSelection(m_cur_page_index);

    if (m_cur_page_index < m_wizard_node->GetChildCount())
    {
        wxGetFrame().SelectNode(m_wizard_node->GetChild(m_cur_page_index), false, true);
    }
}

void MockupWizard::AddPage(MockupWizardPage* page)
{
    m_pages.emplace_back(page);

    m_sizerPage->Add(page, wxSizerFlags(1).Expand());

    if (auto page_sizer = page->GetSizer(); page_sizer)
    {
        auto min_size = page_sizer->GetMinSize();
        min_size.IncBy(5);
        m_largest_nonbmp_page.IncTo(min_size);

        auto bmp = page->GetBitmapPtr();
        if (!bmp->IsOk() && m_bitmap.IsOk())
            bmp = &m_bitmap;
        if (bmp->IsOk())
        {
            min_size.IncBy(bmp->GetScaledSize());
            if (m_wizard_node->prop_as_int(prop_bmp_min_width) > 0 &&
                min_size.x < m_wizard_node->prop_as_int(prop_bmp_min_width))
            {
                min_size.x = m_wizard_node->prop_as_int(prop_bmp_min_width);
            }
            // Add bitmap borders
            min_size.IncBy(5);
        }

        m_largest_page.IncTo(min_size);
        m_window_sizer->SetMinSize(m_largest_page);
    }

    if (m_cur_page_index == tt::npos)
    {
        m_cur_page_index = 0;
    }
    else
        page->Hide();
}

void MockupWizard::AllChildrenAdded()
{
    if (m_bitmap.IsOk())
    {
        if (m_wizard_node->prop_as_int(prop_bmp_placement))
        {
            if (ResizeBitmap(m_bitmap))
            {
                m_statbmp->SetBitmap(m_bitmap);
            }
        }
    }
}

bool MockupWizard::ResizeBitmap(wxBitmap& bmp)
{
    m_bmp_placement = m_wizard_node->prop_as_int(prop_bmp_placement);
    if (!m_bmp_placement || !bmp.IsOk())
        return false;

    // GetScaledWidth() and GetScaledHeight() are new to wxWidgets 3.1.5 and are not currently documented, though they use
    // GetScaleFactor() which is documented. E.g., GetScaledWidth() { return GetWidth() / GetScaleFactor(); }

    auto bmp_width = wxMax(bmp.GetScaledWidth(), m_wizard_node->prop_as_int(prop_bmp_min_width));
    auto bmp_height = wxMax(m_largest_nonbmp_page.y, bmp.GetScaledHeight());

    wxBitmap bitmap(bmp_width, bmp_height);
    wxMemoryDC dc;
    dc.SelectObject(bitmap);
    if (m_wizard_node->HasValue(prop_bmp_background_colour))
        dc.SetBackground(wxBrush(m_wizard_node->prop_as_wxColour(prop_bmp_background_colour)));
    else
        dc.SetBackground(wxBrush(*wxWHITE));
    dc.Clear();

    if (m_bmp_placement & wxWIZARD_TILE)
    {
        wxWizard::TileBitmap(wxRect(0, 0, bmp_width, bmp_height), dc, bmp);
    }
    else
    {
        int x, y;

        if (m_bmp_placement & wxWIZARD_HALIGN_LEFT)
            x = 0;
        else if (m_bmp_placement & wxWIZARD_HALIGN_RIGHT)
            x = bmp_width - bmp.GetScaledWidth();
        else
            x = (bmp_width - bmp.GetScaledWidth()) / 2;

        if (m_bmp_placement & wxWIZARD_VALIGN_TOP)
            y = 0;
        else if (m_bmp_placement & wxWIZARD_VALIGN_BOTTOM)
            y = bmp_height - bmp.GetScaledHeight();
        else
            y = (bmp_height - bmp.GetScaledHeight()) / 2;

        dc.DrawBitmap(bmp, x, y, true);
        dc.SelectObject(wxNullBitmap);
    }

    bmp = bitmap;
    m_size_bmp = bmp.GetScaledSize();

    return true;
}

MockupWizardPage::MockupWizardPage(Node* node, wxObject* parent) : wxPanel(wxStaticCast(parent, wxWindow))
{
    if (node->HasValue(prop_bitmap))
        m_bitmap = node->prop_as_wxBitmap(prop_bitmap);
}
