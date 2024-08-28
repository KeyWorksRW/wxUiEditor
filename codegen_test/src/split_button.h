/////////////////////////////////////////////////////////////////////////////
// Purpose:   Split Button Custom widget (button + menu)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/menu.h>   // wxMenu and wxMenuBar classes
#include <wx/panel.h>  // Base header for wxPanel

namespace wxue_ctrl
{
    class SplitButton : public wxPanel
    {
    public:
        SplitButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos = wxDefaultPosition,
                    const wxSize& size = wxDefaultSize);

        // This is the menu that will be displayed when the drop-down arrow is clicked.
        wxMenu& GetMenu() { return m_menu; }

        // The default width is 20, automatically adjusted if on a High DPI system
        void SetArrowWidth(int width) { m_arrow_btn_width = width; }

        bool Enable(bool enable = true) override;

    protected:
        virtual void OnButton(wxCommandEvent& event) { event.Skip(); }

        void OnLeftButtonDown(wxMouseEvent& event);
        void OnLeftButtonUp(wxMouseEvent& event);
        void OnPaint(wxPaintEvent& WXUNUSED(event));

    private:
        wxString m_label;
        wxMenu m_menu;
        int m_arrow_btn_width;
        int m_btn_flags;
        int m_arrow_flags;
        bool m_is_enabled { true };
    };

}  // namespace wxue_ctrl
