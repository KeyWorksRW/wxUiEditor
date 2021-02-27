//////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "pch.h"

#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/wizard.h>    // wxWizard class: a GUI control presenting the user with a

#include "mockup_wizard.h"

#include "mainframe.h"  // App -- App class
#include "node.h"     // Node class

MockupWizard::MockupWizard(wxWindow* parent, Node* node) : wxPanel(parent)
{
    m_wizard_node = node;
    m_top_sizer = new wxBoxSizer(wxHORIZONTAL);
    // m_top_sizer->SetMinSize(wxSize(270, 270));

    if (node->HasValue("bitmap"))
    {
        auto bitmap = node->prop_as_wxBitmap("bitmap");
        if (bitmap.IsOk())
        {
            auto stat_bmp = new wxStaticBitmap(this, wxID_ANY, bitmap);
            m_top_sizer->Add(stat_bmp, wxSizerFlags().Top());
            m_top_sizer->Add(wxSizerFlags::GetDefaultBorder(), 0, wxSizerFlags().Expand());
            m_size_bmp = bitmap.GetSize();
            m_size_bmp.x += wxSizerFlags::GetDefaultBorder();
        }
    }

    m_sizer_wiz_page = new wxBoxSizer(wxVERTICAL);
    m_top_sizer->Add(m_sizer_wiz_page, wxSizerFlags(1).Expand());

    auto sizer_back_next = new wxBoxSizer(wxHORIZONTAL);

    m_btn_back = new wxButton(this, wxID_BACKWARD, "< &Back");
    m_btn_back->Disable();
    sizer_back_next->Add(m_btn_back, wxSizerFlags().Bottom().Top());

    sizer_back_next->Add(wxSizerFlags::GetDefaultBorder() * 2, 0, wxSizerFlags().Expand());

    m_btn_forward = new wxButton(this, wxID_FORWARD, "&Next >");
    sizer_back_next->Add(m_btn_forward, wxSizerFlags().Bottom().Top());

    auto buttonRow = new wxBoxSizer(wxHORIZONTAL);

    if (node->prop_as_int("extra_style") & wxWIZARD_EX_HELPBUTTON)
    {
        auto btn_help = new wxButton(this, wxID_HELP, "&Help");
        buttonRow->Add(btn_help, wxSizerFlags().Border());
    }

    buttonRow->Add(sizer_back_next, wxSizerFlags().Border());

    auto btn_cancel = new wxButton(this, wxID_CANCEL, "&Cancel");
    buttonRow->Add(btn_cancel, wxSizerFlags().Border());

    auto parent_sizer = new wxBoxSizer(wxVERTICAL);
    parent_sizer->Add(m_top_sizer, wxSizerFlags(1).Expand());

    parent_sizer->Add(0, wxSizerFlags::GetDefaultBorder(), wxSizerFlags().Expand());

    parent_sizer->Add(new wxStaticLine(this), wxSizerFlags().Expand().Border());

    parent_sizer->Add(0, wxSizerFlags::GetDefaultBorder(), wxSizerFlags().Expand());

    parent_sizer->Add(buttonRow, wxSizerFlags().Right());

    auto border = new wxBoxSizer(wxVERTICAL);
    border->Add(parent_sizer, wxSizerFlags(1).Expand().Border());

    SetSizerAndFit(border);

    m_btn_back->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
    m_btn_forward->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
}

void MockupWizard::SetSelection(size_t pageIndex)
{
    auto old_pageIndex = m_cur_page_index;

    if (pageIndex < m_pages.size())
    {
        bool hasPrev = pageIndex > 0;
        bool hasNext = pageIndex < m_pages.size() - 1;

        m_cur_page_index = pageIndex;

        m_btn_back->Enable(hasPrev);

        wxString label = hasNext ? "&Next >" : "&Finish";
        if (label != m_btn_forward->GetLabel())
        {
            m_btn_forward->SetLabel(label);
        }

        m_btn_forward->SetDefault();
    }

    if (old_pageIndex != pageIndex && pageIndex < m_pages.size())
    {
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

    m_pages[m_cur_page_index]->Hide();
    if (event.GetEventObject() == m_btn_forward)
        m_cur_page_index++;
    else
        m_cur_page_index--;
    m_pages[m_cur_page_index]->Show();

    SetSelection(m_cur_page_index);

    if (m_cur_page_index < m_wizard_node->GetChildCount())
    {
        wxGetFrame().SelectNode(m_wizard_node->GetChild(m_cur_page_index), false, true);
    }
}

void MockupWizard::AddPage(wxPanel* page)
{
    if (m_cur_page_index == tt::npos)
    {
        m_cur_page_index = 0;
    }
    else
        page->Hide();

    m_pages.emplace_back(page);

    m_sizer_wiz_page->Add(page, wxSizerFlags(1).Expand());

    // If the page was freshly created, then there will be no sizer
    auto page_sizer = page->GetSizer();
    if (page_sizer)
    {
        auto min_size = page_sizer->GetMinSize();
        min_size.IncBy(m_size_bmp);

        m_top_min_size.IncTo(min_size);
        m_top_sizer->SetMinSize(m_top_min_size);
    }
}
