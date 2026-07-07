/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPanel — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <stdexcept>
#include <string>

#include <wx/filedlg.h>
#include <wx/filename.h>

#include "doc_view_frame.h"
#include "doc_view_panel.h"

#include "archive_handler.h"

#include <cmrc/cmrc.hpp>
CMRC_DECLARE(wxuedocs);

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

DocViewFrame::DocViewFrame(wxWindow* parent, const wxString& zip_path) : DocViewFrame_base(parent)
{
    if (!zip_path.empty())
    {
        std::ignore = OpenArchive(zip_path);
    }
}

// ---------------------------------------------------------------------------
//  OpenArchive
// ---------------------------------------------------------------------------

bool DocViewFrame::OpenArchive(const wxString& zip_path)
{
    if (!m_DocViewPanel->OpenArchive(zip_path))
    {
        return false;
    }

    // Update the title bar to reflect the loaded archive.
    wxFileName fn(zip_path);
    const wxString archive_name = fn.GetFullName();
    SetTitle(wxString::Format("Doc Viewer - %s", archive_name));
    SetStatusText(wxString::Format("Loaded %s", archive_name));

    return true;
}

bool DocViewFrame::IsArchiveOpen() const
{
    return m_DocViewPanel->IsArchiveOpen();
}

// ---------------------------------------------------------------------------
//  Event handlers
// ---------------------------------------------------------------------------

void DocViewFrame::OnClose(wxCloseEvent& event)
{
    // Child mode: hide the frame so the caller (MainFrame) can re-raise it
    // without re-creating it.  Top-level mode: default destroy exits the app.
    if (GetParent() != nullptr)
    {
        Hide();
        event.Veto();
        return;
    }
    event.Skip();  // top-level: proceed with destroy → app exits
}

void DocViewFrame::OnHome([[maybe_unused]] wxCommandEvent& event)
{
    m_DocViewPanel->NavigateHome();
}

void DocViewFrame::OnInfo([[maybe_unused]] wxCommandEvent& event)
{
    if (!wxueArchive.is_open())
    {
        SetStatusText("No archive loaded");
        return;
    }

    const std::string& current_page = wxueArchive.GetCurrentPage();
    if (current_page.empty())
    {
        SetStatusText("No page currently displayed");
        return;
    }

    // Show the current page name and path in the status bar
    SetStatusText(wxString::Format("Current page: %s", wxString::FromUTF8(current_page)));
}

void DocViewFrame::OnOpenArchive([[maybe_unused]] wxCommandEvent& event)
{
    wxFileDialog file_dlg(this, "Open Documentation Archive", wxEmptyString, wxEmptyString,
                          "ZIP archives (*.zip)|*.zip|All files (*.*)|*.*",
                          wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (file_dlg.ShowModal() != wxID_OK)
    {
        return;
    }

    const wxString zip_path = file_dlg.GetPath();
    std::ignore = OpenArchive(zip_path);
}

// ---------------------------------------------------------------------------
//  ResolveDocsZipPath
// ---------------------------------------------------------------------------

wxString ResolveDocsZipPath()
{
    try
    {
        cmrc::embedded_filesystem docs_fs = cmrc::wxuedocs::get_filesystem();
        if (docs_fs.is_file("wxWidgetsDocs.zip"))
        {
            cmrc::file docs_rc = docs_fs.open("wxWidgetsDocs.zip");
            wxFileName temp_fn(wxFileName::GetTempDir(), "wxueditor_wxWidgetsDocs.zip");
            const wxString temp_path = temp_fn.GetFullPath();
            {
                std::ofstream out_file(temp_path.utf8_string(), std::ios::binary | std::ios::trunc);
                out_file.write(docs_rc.begin(), static_cast<std::streamsize>(docs_rc.size()));
            }
            return temp_path;
        }
    }
    catch (...)
    {
        // The embedded archive is guaranteed at build time — rethrow so the
        // caller sees a genuine runtime_error rather than a silent bad path.
        throw;
    }
    // Should be unreachable: archive guaranteed by build_docs.cmake.
    throw std::runtime_error("wxWidgetsDocs.zip not found in embedded filesystem");
}
