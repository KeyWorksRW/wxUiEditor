/////////////////////////////////////////////////////////////////////////////
// Purpose:   Emulate a wxWizard, used for Mockup
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

// A wxWizard derives from wxDialog which makes it unusable as a child of the wxPanel used by our
// Mockup panel. We emulate the functionality here, use similar methods to what that the real
// wxWizard uses (see wxWidgets/src/generic/wizard.cpp).

#include <algorithm>

#include <wx/dcmemory.h>  // wxMemoryDC base header
#include <wx/statbmp.h>   // wxStaticBitmap class interface
#include <wx/statline.h>  // wxStaticLine class interface
#include <wx/wizard.h>    // wxWizard class: a GUI control presenting the user with a
#include <wx/wupdlock.h>  // wxWindowUpdateLocker prevents window redrawing

#include "mockup_wizard.h"

#include "mainframe.h"  // App -- App class
#include "node.h"       // Node class

MockupWizard::MockupWizard(wxWindow* parent, Node* node) :
    wxPanel(parent), m_window_sizer(new wxBoxSizer(wxVERTICAL)),
    m_column_sizer(new wxBoxSizer(wxVERTICAL)), m_wizard_node(node)
{
    m_window_sizer->Add(m_column_sizer, wxSizerFlags(1).Expand().Border());

    CreateBmpPageRow();
    CreateButtonRow();

    SetSizer(m_window_sizer);

    ASSERT(m_btn_prev)

    m_btn_prev->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
    m_btn_next->Bind(wxEVT_BUTTON, &MockupWizard::OnBackOrNext, this);
}

void MockupWizard::CreateBmpPageRow()
{
    m_bmp_page_sizer = new wxBoxSizer(wxHORIZONTAL);
    m_column_sizer->Add(m_bmp_page_sizer, wxSizerFlags(1).Expand().Border());

    if (m_wizard_node->HasValue(prop_bitmap))
    {
        const wxBitmapBundle bundle = m_wizard_node->as_wxBitmapBundle(prop_bitmap);
        m_bitmap = bundle.GetBitmap(bundle.GetPreferredBitmapSizeFor(this));
        if (m_bitmap.IsOk())
        {
            wxSize bmp_size(wxDefaultSize);
            if (m_wizard_node->as_int(prop_bmp_placement) > 0 &&
                m_wizard_node->as_int(prop_bmp_min_width) > 0)
            {
                std::ignore = ResizeBitmap(m_bitmap);
                bmp_size.x = m_wizard_node->as_int(prop_bmp_min_width);
            }
            m_static_bitmap =
                new wxStaticBitmap(this, wxID_ANY, m_bitmap, wxDefaultPosition, bmp_size);
            m_bmp_page_sizer->Add(m_static_bitmap, wxSizerFlags());
            m_bmp_page_sizer->Add(wxSizerFlags::GetDefaultBorder(), 0);

            m_size_bmp = m_bitmap.GetSize();
            m_size_bmp.IncBy(wxSizerFlags::GetDefaultBorder());  // add border size
        }
    }

    m_sizer_page = new wxBoxSizer(wxHORIZONTAL);
    m_bmp_page_sizer->Add(m_sizer_page, wxSizerFlags(1).Expand());
}

void MockupWizard::CreateButtonRow()
{
    auto* static_line = new wxStaticLine(this);
    m_column_sizer->Add(static_line, wxSizerFlags().Expand().Border());

    wxBoxSizer* button_row = new wxBoxSizer(wxHORIZONTAL);

    m_column_sizer->Add(button_row, wxSizerFlags().Right());

    m_btn_prev = new wxButton(this, wxID_BACKWARD, "< &Back");
    m_btn_next = new wxButton(this, wxID_FORWARD, "&Next >");

    wxBoxSizer* back_next_pair = new wxBoxSizer(wxHORIZONTAL);
    button_row->Add(back_next_pair, wxSizerFlags().Border());

    if (m_wizard_node->as_string(prop_extra_style).contains("wxWIZARD_EX_HELPBUTTON"))
    {
        back_next_pair->Add(new wxButton(this, wxID_HELP, "&Help"));
#if defined(__WXMAC__)
        // Put stretchable space between help button and others
        back_next_pair->Add(0, 0, 1, wxALIGN_CENTRE, 0);
#else
        back_next_pair->Add(wxSizerFlags::GetDefaultBorder() * 2, 0, 0, wxEXPAND);
#endif
    }
    back_next_pair->Add(m_btn_prev);
    back_next_pair->Add(wxSizerFlags::GetDefaultBorder() * 2, 0, 0, wxEXPAND);
    back_next_pair->Add(m_btn_next);
    back_next_pair->Add(wxSizerFlags::GetDefaultBorder() * 2, 0, 0, wxEXPAND);
    back_next_pair->Add(new wxButton(this, wxID_CANCEL, "&Cancel"));

    m_button_row_size = m_btn_prev->GetSize();
    m_button_row_size.IncBy(0, static_line->GetSize().y);

    // Add border for static line and buttons
    m_button_row_size.IncBy(0, wxSizerFlags::GetDefaultBorder() * 2);
}

void MockupWizard::SetSelection(size_t page_index)
{
    if (m_pages.empty())
    {
        return;
    }

    wxWindowUpdateLocker const update_lock(this);

    const size_t old_page_index = m_cur_page_index;

    if (page_index < m_pages.size())
    {
        const bool has_prev = page_index > 0;
        const bool has_next = page_index < m_pages.size() - 1;

        m_cur_page_index = page_index;

        m_btn_prev->Enable(has_prev);

        const wxString label = has_next ? "&Next >" : "&Finish";
        if (label != m_btn_next->GetLabel())
        {
            m_btn_next->SetLabel(label);
        }

        m_btn_next->SetDefault();
    }

    if (old_page_index != page_index && page_index < m_pages.size())
    {
        wxBitmap bmp_prev = m_pages[old_page_index]->GetBitmap();

        wxBitmap page_bmp = m_pages[page_index]->GetBitmap();
        if (!page_bmp.IsOk())
        {
            page_bmp = m_bitmap;
        }
        if (!bmp_prev.IsOk())
        {
            bmp_prev = m_bitmap;
        }

        if (!page_bmp.IsSameAs(bmp_prev) && m_static_bitmap)
        {
            m_static_bitmap->SetBitmap(page_bmp);
        }

        m_pages[old_page_index]->Hide();
        m_pages[page_index]->Show();
        m_cur_page_index = page_index;
    }

    Fit();
    Layout();
}

// TODO: [Randalphwa - 04-16-2026] This function is called, but does nothing. Figure out what it's
// supposed to do and implement it, or remove it and the call to it if it's not needed.
void MockupWizard::extracted()
{
    return;
}

void MockupWizard::OnBackOrNext(wxCommandEvent& event)
{
    if (m_cur_page_index == wxue::npos || m_pages.empty())
    {
        return;
    }

    size_t new_index = m_cur_page_index;
    if (event.GetId() == wxID_FORWARD)
    {
        if (m_cur_page_index + 1 >= m_pages.size())
        {
            return;
        }
        new_index = m_cur_page_index + 1;
    }
    else
    {
        if (m_cur_page_index == 0)
        {
            extracted();
            return;
        }
        new_index = m_cur_page_index - 1;
    }

    SetSelection(new_index);

    if (new_index < m_wizard_node->get_ChildCount())
    {
        wxGetFrame().SelectNode(m_wizard_node->get_Child(new_index), evt_flags::fire_event);
    }
}

void MockupWizard::AddPage(MockupWizardPage* page)
{
    m_pages.emplace_back(page);

    m_sizer_page->Add(page, wxSizerFlags(1).Expand());

    if (wxSizer* page_sizer = page->GetSizer(); page_sizer)
    {
        wxSize min_size = page_sizer->GetMinSize();
        min_size.IncBy(wxSizerFlags::GetDefaultBorder());
        m_largest_nonbmp_page.IncTo(min_size);

        const wxBitmap* page_bitmap = page->GetBitmapPtr();
        if (!page_bitmap->IsOk() && m_bitmap.IsOk())
        {
            page_bitmap = &m_bitmap;
        }
        if (page_bitmap->IsOk())
        {
            wxSize bmp_size = page_bitmap->GetScaledSize();
            if (m_wizard_node->as_int(prop_bmp_min_width) > 0 &&
                bmp_size.x < m_wizard_node->as_int(prop_bmp_min_width))
            {
                bmp_size.x = m_wizard_node->as_int(prop_bmp_min_width);
            }
            min_size.IncBy(bmp_size.x, 0);
        }

        m_largest_page.IncTo(min_size);
        m_largest_page.IncBy(0, m_button_row_size.y);
        m_window_sizer->SetMinSize(m_largest_page);
    }

    if (m_cur_page_index == wxue::npos)
    {
        m_cur_page_index = 0;
    }
    else
    {
        page->Hide();
    }
}

void MockupWizard::AllChildrenAdded()
{
    if (m_bitmap.IsOk())
    {
        if (m_wizard_node->as_int(prop_bmp_placement))
        {
            std::expected<void, std::string_view> result = ResizeBitmap(m_bitmap);
#if !defined(NDEBUG) || defined(INTERNAL_TESTING)
            if (!result)
            {
                FAIL_MSG(wxString(result.error()));
            }
#endif
            if (result && m_static_bitmap)
            {
                m_static_bitmap->SetBitmap(m_bitmap);
            }
        }
    }
}

std::expected<void, std::string_view> MockupWizard::ResizeBitmap(wxBitmap& source_bmp)
{
    m_bmp_placement = m_wizard_node->as_int(prop_bmp_placement);
    if (!m_bmp_placement)
    {
        return std::unexpected("prop_bmp_placement has not been set");
    }
    if (!source_bmp.IsOk())
    {
        return std::unexpected("source bitmap is invalid");
    }

    // GetScaledWidth() and GetScaledHeight() are new to wxWidgets 3.2.x and are not currently
    // documented, though they use GetScaleFactor() which is documented. E.g., GetScaledWidth() {
    // return GetWidth() / GetScaleFactor(); }

    const double bmp_width =
        std::max<double>(source_bmp.GetScaledWidth(), m_wizard_node->as_int(prop_bmp_min_width));
    const double bmp_height =
        std::max<double>(m_largest_nonbmp_page.y, source_bmp.GetScaledHeight());

    wxBitmap resized_bmp(static_cast<int>(bmp_width), static_cast<int>(bmp_height));
    wxMemoryDC mem_dc;
    mem_dc.SelectObject(resized_bmp);
    if (m_wizard_node->HasValue(prop_bmp_background_colour))
    {
        mem_dc.SetBackground(wxBrush(m_wizard_node->as_wxColour(prop_bmp_background_colour)));
    }
    else
    {
        mem_dc.SetBackground(wxBrush(*wxWHITE));
    }
    mem_dc.Clear();

    if (m_bmp_placement & wxWIZARD_TILE)
    {
        wxWizard::TileBitmap(
            wxRect(0, 0, static_cast<int>(bmp_width), static_cast<int>(bmp_height)), mem_dc,
            source_bmp);
    }
    else
    {
        int pos_x = 0;
        int pos_y = 0;

        if (m_bmp_placement & wxWIZARD_HALIGN_LEFT)
        {
            pos_x = 0;
        }
        else if (m_bmp_placement & wxWIZARD_HALIGN_RIGHT)
        {
            pos_x = static_cast<int>(bmp_width - source_bmp.GetScaledWidth());
        }
        else
        {
            pos_x = static_cast<int>((bmp_width - source_bmp.GetScaledWidth()) / 2);
        }

        if (m_bmp_placement & wxWIZARD_VALIGN_TOP)
        {
            pos_y = 0;
        }
        else if (m_bmp_placement & wxWIZARD_VALIGN_BOTTOM)
        {
            pos_y = static_cast<int>(bmp_height - source_bmp.GetScaledHeight());
        }
        else
        {
            pos_y = static_cast<int>((bmp_height - source_bmp.GetScaledHeight()) / 2);
        }

        mem_dc.DrawBitmap(source_bmp, pos_x, pos_y, true);
    }

    mem_dc.SelectObject(wxNullBitmap);
    source_bmp = resized_bmp;
    m_size_bmp = source_bmp.GetScaledSize();

    return {};
}

MockupWizardPage::MockupWizardPage(Node* node, wxObject* parent) :
    wxPanel(wxStaticCast(parent, wxWindow))
{
    if (node->HasValue(prop_bitmap))
    {
        const wxBitmapBundle bundle = node->as_wxBitmapBundle(prop_bitmap);
        m_bitmap = bundle.GetBitmap(bundle.GetPreferredBitmapSizeFor(this));
    }
}
