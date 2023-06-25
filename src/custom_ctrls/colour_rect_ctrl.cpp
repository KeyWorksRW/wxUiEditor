/////////////////////////////////////////////////////////////////////////////
// Purpose:   Control that displays a solid color
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/dcclient.h>  // wxClientDC base header

#include "colour_rect_ctrl.h"  // ColourRectCtrl

using namespace wxue_ctrl;

ColourRectCtrl::ColourRectCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style) :
    wxControl(parent, id, pos, size, style)
{
    Bind(wxEVT_PAINT, &ColourRectCtrl::OnPaint, this);
}

void ColourRectCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    auto size = GetSize();

    wxBrush brush(m_clr);
    dc.SetBrush(brush);
    dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
}
