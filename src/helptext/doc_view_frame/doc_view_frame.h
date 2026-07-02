/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPane — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>

#include <wx/frame.h>

class DocViewPane;
class wxStatusBar;

// wxFrame host for DocViewPane.
//
// Supports two operational modes:
//   Top-level  (parent == nullptr): launched via --docview CLI; title
//              "wxUiEditor Doc Viewer"; Close() terminates the app.
//   Child      (parent != nullptr): launched from MainFrame; title
//              "Documentation"; Close() destroys the frame only.
//
// Menu bar:
//   File        — &Open Archive...  (Ctrl+O)
//   Navigation  — &Home             (Ctrl+H)
//               — &Find...          (Ctrl+F)
//
// Status bar:
//   Single field showing the current archive page name, updated via
//   DocViewPane::m_on_status_update.
class DocViewFrame : public wxFrame
{
public:
    DocViewFrame() = default;
    ~DocViewFrame() override = default;

    DocViewFrame(const DocViewFrame&) = delete;
    DocViewFrame& operator=(const DocViewFrame&) = delete;
    DocViewFrame(DocViewFrame&&) = delete;
    DocViewFrame& operator=(DocViewFrame&&) = delete;

    // Construct and show the frame.  Pass zip_path as an empty path to start
    // without an archive (the user can open one via File > Open Archive).
    DocViewFrame(wxWindow* parent, const std::filesystem::path& zip_path);

    // Open (or re-open) a documentation ZIP archive.
    // Updates the title bar to "Doc Viewer - <filename>" on success.
    // Returns false when the archive could not be opened.
    [[nodiscard]] bool OpenArchive(const std::filesystem::path& zip_path);

    // True when an archive has been successfully opened.
    [[nodiscard]] bool IsArchiveOpen() const;

private:
    // Menu event handlers
    void OnOpenArchive(wxCommandEvent& event);
    void OnHome(wxCommandEvent& event);
    void OnFind(wxCommandEvent& event);

    // Close handler: top-level mode destroys and exits; child mode hides the frame.
    void OnClose(wxCloseEvent& event);

    DocViewPane* m_doc_view_pane { nullptr };
    wxStatusBar* m_status_bar { nullptr };
    bool m_archive_open { false };
};
