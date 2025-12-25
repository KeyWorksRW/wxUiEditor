/////////////////////////////////////////////////////////////////////////////
// Purpose:   MainFrame status bar functions
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "mainframe.h"

/*
 * UGLY HACK ALERT!
 *
 * Unlike Menu and Toolbar help where you can specify what statusbar panel to send text to,
 * wxPropertyGridManager will always send help text to statusbar panel #0. Because that text is
 * quite long, we want it to be a variable length field. We also want it to keep displaying for as
 * long at the property is selected. Ideally, we would display regular status text in a fixed panel
 * on the left along with toolbar and menu help, and the propertygrid help text in the right
 * variable length panel. But you can't do that normally because of the fixed panel number
 * wxPropertyGridManager writes to.
 *
 * To get around this, we create three panels. Panel #0 is the smallest size possible and uses a
 * flat display so that it doesn't actually look like a panel. Panel #1 is a fixed width and used
 * for general status bar text along with toolbar and menu help text. Panel #2 is the variable width
 * which means it will normally be the largest.
 *
 * Next we create a custom statusbar and when it is notified that text was sent to panel #0 it
 * copies it to panel #2 and erases the text in panel #0.
 *
 * As long as we have a custom bar anyway, we add a setText method that takes a const wxString&.
 */

class ueStatusBar : public wxStatusBar
{
public:
    ueStatusBar(wxWindow* parent, wxWindowID id = wxID_ANY, long style = wxSTB_DEFAULT_STYLE,
                const wxString& name = wxStatusBarNameStr)
    {
        Create(parent, id, style, name);
    }

    auto DoUpdateStatusText(int number) -> void override
    {
        ASSERT(m_panes.size() > static_cast<size_t>(number));
        const auto text = GetStatusText(number);
        if (text.size() && number == 0)
        {
            SetStatusText(text, 2);
            SetStatusText(wxEmptyString, 0);
        }
        wxStatusBar::DoUpdateStatusText(number);
    }

    auto setText(const wxString& txt, int pane = 1) -> void { SetStatusText(txt, pane); }
};

auto MainFrame::OnCreateStatusBar(int number, long style, wxWindowID id, const wxString& name)
    -> wxStatusBar*
{
    m_statBar = new ueStatusBar(this, id, style, name);
    m_statBar->SetFieldsCount(number);
    {
        const std::array<int, STATUS_PANELS> styles = { wxSB_FLAT, wxSB_NORMAL, wxSB_NORMAL };
        m_statBar->SetStatusStyles(STATUS_PANELS, styles.data());
    }

    return m_statBar;
}

auto MainFrame::setStatusText(const wxString& txt, int pane) -> void
{
    if (m_statBar)
    {
        m_statBar->setText(txt, pane);
    }
}
