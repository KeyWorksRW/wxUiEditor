////////////////////////////////////////////////////////////////////////////////
// Code generated by wxUiEditor -- see https://github.com/KeyWorksRW/wxUiEditor/
//
// DO NOT EDIT THIS FILE! Your changes will be lost if it is re-generated!
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/bitmap.h>
#include <wx/button.h>
#include <wx/commandlinkbutton.h>
#include <wx/dialog.h>
#include <wx/event.h>
#include <wx/gdicmn.h>
#include <wx/icon.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/stattext.h>
#include <wx/tglbtn.h>

class DlgMultiTestBase : public wxDialog
{
public:
    DlgMultiTestBase(wxWindow* parent, wxWindowID id = wxID_ANY,
        const wxString& title = wxString::FromUTF8("Widgets Testing"),
        const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxSize(600, 800),
        long style = wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER);

protected:

    // Class member variables

    wxButton* m_btn;
    wxButton* m_btn_2;
    wxButton* m_btn_4;
    wxButton* m_btn_bitmaps;
    wxCommandLinkButton* m_btn_5;
    wxNotebook* m_notebook;
    wxStaticText* m_staticText;
    wxStaticText* m_staticText_3;
    wxStaticText* m_staticText_4;
    wxStaticText* m_staticText_5;
    wxToggleButton* m_toggleBtn;
};
