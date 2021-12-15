/////////////////////////////////////////////////////////////////////////////
// Purpose:   Control that displays a solid color
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/colour.h>   // wxColourBase definition
#include <wx/control.h>  // wxControl common interface

// Can we use a wxTextValidator?

namespace wxue_ctrl
{
    class ColourRectCtrl : public wxControl
    {
    public:
        ColourRectCtrl(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxDefaultSize, long style = wxBORDER_SIMPLE);

        void SetColour(wxColour clr) { m_clr = clr; }

    protected:
        void OnPaint(wxPaintEvent& WXUNUSED(event));

    private:
        wxColour m_clr { *wxBLACK };
    };

}  // namespace wxue_ctrl
