/////////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard, used for Mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// A wxWizard derives from wxDialog which makes it unusable as a child of the wxPanel used by our Mockup panel. We emulate
// the functionality here, use similar methods to what that the real wxWizard uses (see wxWidgets/src/generic/wizard.cpp).

#include "pch.h"

#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/wizard.h>    // wxWizard class: a GUI control presenting the user with a

#include "mockup_wizard.h"

#include "mainframe.h"  // App -- App class
#include "node.h"       // Node class

MockupWizard::MockupWizard(wxWindow* parent, Node* node) : wxPanel(parent)
{
    m_wizard_node = node;
    m_window_sizer = new wxBoxSizer(wxVERTICAL);

    m_column_sizer = new wxBoxSizer(wxVERTICAL);
    m_window_sizer->Add(m_column_sizer, wxSizerFlags(1).Expand());
    AddBitmapRow(m_column_sizer);

    m_column_sizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border());
    m_column_sizer->Add(0, 5, 0, wxEXPAND);

    AddButtonRow(m_column_sizer);

    SetSizer(m_window_sizer);

    m_btnPrev->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
    m_btnNext->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
}

void MockupWizard::AddBitmapRow(wxBoxSizer* mainColumn)
{
    m_sizerBmpAndPage = new wxBoxSizer(wxHORIZONTAL);
    mainColumn->Add(m_sizerBmpAndPage, 1, wxEXPAND);
    mainColumn->Add(0, 5, 0, wxEXPAND);

    if (m_wizard_node->HasValue(prop_bitmap))
    {
        m_bitmap = m_wizard_node->prop_as_wxBitmap(prop_bitmap);
        if (m_bitmap.IsOk())
        {
            wxSize bitmapSize(wxDefaultSize);
            if (m_wizard_node->prop_as_int(prop_bmp_placement) > 0)
            {
                bitmapSize.x = m_wizard_node->prop_as_int(prop_bmp_placement);
            }
            m_statbmp = new wxStaticBitmap(this, wxID_ANY, m_bitmap, wxDefaultPosition, bitmapSize);
            m_sizerBmpAndPage->Add(m_statbmp, wxSizerFlags());
            m_sizerBmpAndPage->Add(5, 0, 0, wxEXPAND);
        }
    }

    m_sizerPage = new wxBoxSizer(wxHORIZONTAL);
    m_sizerBmpAndPage->Add(m_sizerPage, wxSizerFlags());
}

void MockupWizard::AddButtonRow(wxBoxSizer* mainColumn)
{
    wxBoxSizer* buttonRow = new wxBoxSizer(wxHORIZONTAL);

    mainColumn->Add(buttonRow, 0, wxALIGN_RIGHT);

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

    Fit();
    Layout();
}

void MockupWizard::AddPage(MockupWizardPage* page)
{
    if (m_cur_page_index == tt::npos)
    {
        m_cur_page_index = 0;
    }
    else
        page->Hide();

    m_pages.emplace_back(page);

    m_sizerPage->Add(page, wxSizerFlags(1).Expand());

    if (auto page_sizer = page->GetSizer(); page_sizer)
    {
        auto min_size = page_sizer->GetMinSize();
        min_size.IncBy(m_size_bmp);

        m_top_min_size.IncTo(min_size);
        m_window_sizer->SetMinSize(m_top_min_size);
    }
}

MockupWizardPage::MockupWizardPage(Node* node, wxObject* parent) : wxPanel(wxStaticCast(parent, wxWindow))
{
    if (node->HasValue(prop_bitmap))
        m_bitmap = node->prop_as_wxBitmap(prop_bitmap);
}
