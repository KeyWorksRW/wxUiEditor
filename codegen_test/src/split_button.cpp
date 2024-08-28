/////////////////////////////////////////////////////////////////////////////
// Purpose:   Split Button Custom widget (button + menu)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>    // wxButtonBase class
#include <wx/dcclient.h>  // wxClientDC base header
#include <wx/event.h>     // Event classes
#include <wx/renderer.h>  // wxRendererNative class declaration

#include "split_button.h"

#ifndef wxCONTROL_NONE
    #define wxCONTROL_NONE 0  // This was defined in wxWidgets 3.1
#endif

using namespace wxue_ctrl;

SplitButton::SplitButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size) :
    wxPanel(parent, id, pos, size, wxBORDER_NONE | wxTAB_TRAVERSAL), m_label(label)
{
    m_arrow_btn_width = FromDIP(20);
    m_btn_flags = wxCONTROL_NONE;
    m_arrow_flags = wxCONTROL_NONE;

    if (size == wxDefaultSize)
    {
        auto defaultSize = wxButton::GetDefaultSize();

        auto textSize = GetTextExtent(m_label);
        textSize.SetWidth(textSize.GetWidth() + m_arrow_btn_width + FromDIP(20));
        SetMinSize(wxSize(textSize.GetWidth(), defaultSize.GetHeight()));
    }

    Bind(wxEVT_PAINT, &SplitButton::OnPaint, this);
    Bind(wxEVT_LEFT_UP, &SplitButton::OnLeftButtonUp, this);
    Bind(wxEVT_LEFT_DOWN, &SplitButton::OnLeftButtonDown, this);

    Bind(wxEVT_ENTER_WINDOW,
         [this](wxMouseEvent& event)
         {
             m_btn_flags = wxCONTROL_CURRENT;
             m_arrow_flags = wxCONTROL_CURRENT;
             Refresh();
             event.Skip();
         });

    Bind(wxEVT_LEAVE_WINDOW,
         [this](wxMouseEvent& event)
         {
             m_btn_flags = wxCONTROL_NONE;
             m_arrow_flags = wxCONTROL_NONE;
             Refresh();
             event.Skip();
         });

    Bind(wxEVT_SET_FOCUS,
         [this](wxFocusEvent& event)
         {
             m_btn_flags = wxCONTROL_CURRENT;
             m_arrow_flags = wxCONTROL_CURRENT;
             Refresh();
             event.Skip();
         });

    Bind(wxEVT_KILL_FOCUS,
         [this](wxFocusEvent& event)
         {
             m_btn_flags = wxCONTROL_NONE;
             m_arrow_flags = wxCONTROL_NONE;
             Refresh();
             event.Skip();
         });
}

void SplitButton::OnLeftButtonUp(wxMouseEvent& event)
{
    m_btn_flags = wxCONTROL_NONE;
    m_arrow_flags = wxCONTROL_NONE;

    Refresh();

    auto pos = event.GetPosition();
    if (pos.x < (GetSize().GetWidth() - m_arrow_btn_width))
    {
        auto EventHandler = GetEventHandler();
        if (EventHandler)
        {
            EventHandler->CallAfter(
                [=]()
                {
                    wxCommandEvent evt(wxEVT_BUTTON, GetId());
                    evt.SetEventObject(this);
                    GetEventHandler()->ProcessEvent(evt);
                });
        }
    }

    event.Skip();
}

void SplitButton::OnLeftButtonDown(wxMouseEvent& event)
{
    auto pos = event.GetPosition();
    if (pos.x >= (GetSize().GetWidth() - m_arrow_btn_width))
    {
        m_btn_flags = wxCONTROL_NONE;
        m_arrow_flags = wxCONTROL_PRESSED;
        Refresh();

        PopupMenu(&m_menu, { 0, GetSize().GetHeight() });

        m_arrow_flags = wxCONTROL_NONE;
        Refresh();
    }
    else
    {
        m_btn_flags = wxCONTROL_PRESSED;
        m_arrow_flags = wxCONTROL_PRESSED;
        Refresh();
    }

    event.Skip();
}

void SplitButton::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    auto size = GetSize();
    int width = size.GetWidth() - m_arrow_btn_width;

    wxRect rect { 0, 0, width + FromDIP(2), size.GetHeight() };
    wxRendererNative::Get().DrawPushButton(this, dc, rect, m_btn_flags);

    SetForegroundColour(m_is_enabled ? GetForegroundColour() : GetForegroundColour().MakeDisabled());

    rect.y += (size.GetHeight() - GetCharHeight()) / 2;
    dc.DrawLabel(m_label, rect, wxALIGN_CENTER_HORIZONTAL);

    rect.x = width - FromDIP(2);
    rect.y = 0;
    rect.width = m_arrow_btn_width;

    wxRendererNative::Get().DrawPushButton(this, dc, rect, m_arrow_flags);
    wxRendererNative::Get().DrawDropArrow(this, dc, rect, m_arrow_flags);
}

bool SplitButton::Enable(bool enable)
{
    if (m_is_enabled == enable)
        return false;  // nothing changed
    else
        m_is_enabled = enable;

    wxPanel::Enable(enable);

    if (enable)
    {
        m_btn_flags = wxCONTROL_NONE;
        m_arrow_flags = wxCONTROL_NONE;
    }
    else
    {
        m_btn_flags = wxCONTROL_DISABLED;
        m_arrow_flags = wxCONTROL_DISABLED;
    }
    Refresh();

    return true;
}
