/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPane — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "doc_view_frame/doc_view_frame.h"

#include <filesystem>
#include <string>

#include <wx/filedlg.h>
#include <wx/menu.h>
#include <wx/statusbr.h>

#include "doc_view_panel/doc_view_pane.h"

// ---------------------------------------------------------------------------
//  IDs
// ---------------------------------------------------------------------------

namespace
{
    constexpr int ID_OPEN_ARCHIVE = wxID_OPEN;
    constexpr int ID_NAV_HOME = wxID_HOME;
    constexpr int ID_NAV_FIND = wxID_FIND;
}  // namespace

// ---------------------------------------------------------------------------
//  DocViewFrame constructor
// ---------------------------------------------------------------------------

DocViewFrame::DocViewFrame(wxWindow* parent, const std::filesystem::path& zip_path)
{
    const bool is_top_level = (parent == nullptr);
    const wxString title =
        is_top_level ? wxString("wxUiEditor Doc Viewer") : wxString("Documentation");

    wxFrame::Create(parent, wxID_ANY, title, wxDefaultPosition, wxSize(1024, 768));

    // ----- Menu bar -----
    wxMenuBar* const menu_bar = new wxMenuBar;

    wxMenu* const file_menu = new wxMenu;
    file_menu->Append(ID_OPEN_ARCHIVE, "&Open Archive...\tCtrl+O",
                      "Open a documentation ZIP archive");

    wxMenu* const nav_menu = new wxMenu;
    nav_menu->Append(ID_NAV_HOME, "&Home\tCtrl+H", "Navigate to the home page");
    nav_menu->Append(ID_NAV_FIND, "&Find...\tCtrl+F", "Find text in the current page");

    menu_bar->Append(file_menu, "&File");
    menu_bar->Append(nav_menu, "&Navigation");
    SetMenuBar(menu_bar);

    // ----- Status bar -----
    m_status_bar = CreateStatusBar(1);

    // ----- Content pane -----
    m_doc_view_pane = new DocViewPane;
    m_doc_view_pane->Create(this);

    // Wire status-bar callback: whenever DocViewPane changes page it calls us.
    m_doc_view_pane->m_on_status_update = [this](const std::string& page_name)
    {
        if (m_status_bar)
        {
            m_status_bar->SetStatusText(wxString::FromUTF8(page_name));
        }
    };

    // ----- Event bindings -----
    Bind(wxEVT_MENU, &DocViewFrame::OnOpenArchive, this, ID_OPEN_ARCHIVE);
    Bind(wxEVT_MENU, &DocViewFrame::OnHome, this, ID_NAV_HOME);
    Bind(wxEVT_MENU, &DocViewFrame::OnFind, this, ID_NAV_FIND);
    Bind(wxEVT_CLOSE_WINDOW, &DocViewFrame::OnClose, this);

    // Ctrl+F via CHAR_HOOK — more reliable than an accelerator table with
    // controls that capture keystrokes at a low level (wxHtmlWindow, etc.)
    Bind(wxEVT_CHAR_HOOK,
         [this](wxKeyEvent& key_event)
         {
             if (key_event.ControlDown() && key_event.GetKeyCode() == static_cast<int>('F'))
             {
                 wxCommandEvent dummy_event;
                 OnFind(dummy_event);
                 return;
             }
             key_event.Skip();
         });

    // ----- Initial archive -----
    if (!zip_path.empty())
    {
        std::ignore = OpenArchive(zip_path);
    }
}

// ---------------------------------------------------------------------------
//  DocViewFrame::OpenArchive
// ---------------------------------------------------------------------------

bool DocViewFrame::OpenArchive(const std::filesystem::path& zip_path)
{
    if (!m_doc_view_pane)
    {
        return false;
    }

    m_doc_view_pane->OpenArchive(zip_path);
    m_archive_open = true;

    // Update the title bar to reflect the loaded archive.
    const wxString archive_name = wxString::FromUTF8(zip_path.filename().string());
    SetTitle(wxString::Format("Doc Viewer - %s", archive_name));

    return true;
}

// ---------------------------------------------------------------------------
//  DocViewFrame::IsArchiveOpen
// ---------------------------------------------------------------------------

bool DocViewFrame::IsArchiveOpen() const
{
    return m_archive_open;
}

// ---------------------------------------------------------------------------
//  Menu handlers
// ---------------------------------------------------------------------------

void DocViewFrame::OnOpenArchive([[maybe_unused]] wxCommandEvent& event)
{
    wxFileDialog file_dlg(this, "Open Documentation Archive", wxEmptyString, wxEmptyString,
                          "ZIP archives (*.zip)|*.zip|All files (*.*)|*.*",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (file_dlg.ShowModal() != wxID_OK)
    {
        return;
    }

    const std::filesystem::path zip_path(file_dlg.GetPath().ToStdWstring());
    std::ignore = OpenArchive(zip_path);
}

void DocViewFrame::OnHome([[maybe_unused]] wxCommandEvent& event)
{
    if (m_doc_view_pane)
    {
        m_doc_view_pane->OnHome();
    }
}

void DocViewFrame::OnFind([[maybe_unused]] wxCommandEvent& event)
{
    if (m_doc_view_pane)
    {
        m_doc_view_pane->OnFind();
    }
}

void DocViewFrame::OnClose(wxCloseEvent& close_event)
{
    // Child mode: hide the frame so the caller (MainFrame) can re-raise it
    // without re-creating it.  Top-level mode: default destroy exits the app.
    if (GetParent() != nullptr)
    {
        Hide();
        close_event.Veto();
        return;
    }
    close_event.Skip();  // top-level: proceed with destroy → app exits
}
