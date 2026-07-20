/////////////////////////////////////////////////////////////////////////////
// Purpose:   Control that displays a solid color
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-12-2026]

#include <wx/dcclient.h>  // wxClientDC base header

#include "colour_rect_ctrl.h"  // ColourRectCtrl

using namespace wxue_ctrl;

ColourRectCtrl::ColourRectCtrl(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                               const wxSize& size, long style) :
    wxControl(parent, id, pos, size, style)
{
    Bind(wxEVT_PAINT, &ColourRectCtrl::OnPaint, this);
}

void ColourRectCtrl::OnPaint(wxPaintEvent& /* event unused */)
{
    wxPaintDC paint_dc(this);
    const wxSize size = GetSize();

    const wxBrush brush(m_clr);
    paint_dc.SetBrush(brush);
    paint_dc.DrawRectangle(0, 0, size.GetWidth(), size.GetHeight());
}
