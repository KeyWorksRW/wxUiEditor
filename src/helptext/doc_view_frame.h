/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPanel — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/string.h>

#include "doc_view_frame_base.h"

// Resolve the path to the wxWidgets docs ZIP. Materializes the CMakeRC-embedded
// archive to a temp file and returns its path. Throws on failure (archive is
// guaranteed to be embedded at build time).
wxString ResolveDocsZipPath();
// DocViewFrame_base host with two operational modes:
//   Top-level  (parent == nullptr): launched via --docview CLI; title
//              "wxUiEditor Doc Viewer"; Close() terminates the app.
//   Child      (parent != nullptr): launched from MainFrame; title
//              "Documentation"; Close() destroys the frame only.
class DocViewFrame : public DocViewFrame_base
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
    DocViewFrame(wxWindow* parent, const wxString& zip_path);

    // Open (or re-open) a documentation ZIP archive.
    // Updates the title bar to "Doc Viewer - <filename>" on success.
    // Returns false when the archive could not be opened.
    [[nodiscard]] bool OpenArchive(const wxString& zip_path);

    // True when an archive has been successfully opened.
    [[nodiscard]] bool IsArchiveOpen() const;

protected:
    // Overrides for DocViewFrame_base pure virtual event handlers
    void OnClose(wxCloseEvent& event) override;
    void OnHome(wxCommandEvent& event) override;
    void OnInfo(wxCommandEvent& event) override;
    void OnOpenArchive(wxCommandEvent& event) override;
};
