/////////////////////////////////////////////////////////////////////////////
// Purpose:   Dialog to display if wxUiEditor is launched with no arguments
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/button.h>        // wxButtonBase class
#include <wx/html/htmlwin.h>  // wxHtmlWindow class for parsing & displaying HTML
#include <wx/sizer.h>         // provide wxSizer class for layout

#include "startup.h"  // CStartup

#include "bitmaps.h"    // Map of bitmaps accessed by name
#include "mainapp.h"    // App -- App class
#include "mainframe.h"  // MainFrame -- Main window frame

// clang-format off
static const char* txtContents = {
    "<html>"
    "<body>"
    "<font size=\"4\">"
    "Click a link below to open a previously opened project, convert a different type of project or create a new empty project."
    "<h2>Open Recent Project</h2>"
    "%recent%"
    "<par><br>"
    "Open an <b><a href=\"$existing\">existing</a></b> project."
    "<h2>Create New Project</h2>"
    "<b><a href=\"$convert\">Import</a></b> from a different type of project (<b>wxFormBuilder</b>, <b>wxGlade</b>, <b>wxSmith</b>, <b>XRC</b> or <b>Windows Resource</b>).<br>"
    "<br>"
    "Create an <b><a href=\"$empty\">empty</a></b> project."
    "</font>"
    "</body>"
    "</html>"
};
// clang-format on

// Class that causes links to be launched in an external browser.
class CStartupHtmlWindow : public wxHtmlWindow
{
public:
    CStartupHtmlWindow(CStartup* caller) :
        wxHtmlWindow(caller, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                     wxHW_SCROLLBAR_NEVER | wxHW_NO_SELECTION | wxRAISED_BORDER)
    {
        m_Startup = caller;
    }

    void OnLinkClicked(const wxHtmlLinkInfo& link) override { m_Startup->LinkCommand(link.GetHref()); }

private:
    CStartup* m_Startup;
};

CStartup::CStartup() :
    // wxDialog(nullptr, wxID_ANY, txtAppname, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
    wxDialog(nullptr, wxID_ANY, txtAppname, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE)
{
    SetIcon(GetIconImage("logo32"));

    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto htmlWindow = new CStartupHtmlWindow(this);

    auto& history = wxGetApp().GetMainFrame()->GetFileHistory();

    ttlib::cstr ReplaceRecent;
    ttlib::cstr file;

    for (size_t pos = 0; pos < history.GetCount(); ++pos)
    {
        file.utf(history.GetHistoryFile(pos).wx_str());
        file.backslashestoforward();
        ttlib::cstr path = file;
        path.remove_filename();
        ReplaceRecent << "<b><a href=\"" << file << "\">" << file.filename() << "</a></b>"
                      << " (" << path << ")<br>";
    }

#if defined(_DEBUG)
    auto append_history_ptr = wxGetFrame().GetAppendImportHistory();
    if (append_history_ptr->GetCount())
        ReplaceRecent << "<br>";

    for (size_t pos = 0; pos < append_history_ptr->GetCount(); ++pos)
    {
        file.utf(append_history_ptr->GetHistoryFile(pos).wx_str());
        file.backslashestoforward();
        ttlib::cstr path = file;
        path.remove_filename();
        ReplaceRecent << "<b><a href=\"" << file << "\">" << file.filename() << "</a></b>"
                      << " (" << path << ")<br>";
    }
#endif  // _DEBUG

    ttlib::cstr page(txtContents);
    if (ReplaceRecent.size())
        page.Replace("%recent%", ReplaceRecent);
    else
        page.Replace("%recent%", "There are no recently opened projects.");

    htmlWindow->SetPage(page);
#if defined(_DEBUG)
    // Debug has another row of up to 10 items, so add some extra space.
    htmlWindow->SetInitialSize(
        ConvertPixelsToDialog(wxSize(1000 > GetBestSize().x ? 1000 : -1, 1200 > GetBestSize().x ? 1200 : -1)));
#else
    htmlWindow->SetInitialSize(
        ConvertPixelsToDialog(wxSize(1000 > GetBestSize().x ? 1000 : -1, 1000 > GetBestSize().x ? 1000 : -1)));
#endif  // _DEBUG
    // htmlWindow->SetMinSize(wxSize(600, 500));

    sizer->Add(htmlWindow, wxSizerFlags(1).Expand().Border(wxALL, 10));
    sizer->Add(new wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags().Center().Border(wxBOTTOM, 8));

    SetSizerAndFit(sizer);
    Center();
}

void CStartup::LinkCommand(const wxString& cmd)
{
    if (cmd == "$empty")
    {
        m_cmdType = START_EMPTY;
    }
    else if (cmd == "$existing")
    {
        m_cmdType = START_OPEN;
    }
    else if (cmd == "$convert")
    {
        m_cmdType = START_CONVERT;
    }
    else
    {
        m_cmdType = START_MRU;
        m_mruFile.utf(cmd.wx_str());
    }
    EndModal(wxID_OK);
}
