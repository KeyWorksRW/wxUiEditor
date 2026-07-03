/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPane — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../internal/doc_view_frame_base.h"

class wxListBox;
class wxObject;
class wxTextCtrl;

// Direct base and derived classes for one class, loaded from data/inheritance.json.
struct InheritEntry
{
    std::vector<std::string> bases;
    std::vector<std::string> derived;
};

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
    DocViewFrame(wxWindow* parent, const std::filesystem::path& zip_path);

    // Open (or re-open) a documentation ZIP archive.
    // Updates the title bar to "Doc Viewer - <filename>" on success.
    // Returns false when the archive could not be opened.
    [[nodiscard]] bool OpenArchive(const std::filesystem::path& zip_path);

    // True when an archive has been successfully opened.
    [[nodiscard]] bool IsArchiveOpen() const;

protected:
    // Overrides for DocViewFrame_base pure virtual event handlers
    void OnClose(wxCloseEvent& event) override;
    void OnDblClickListBox(wxCommandEvent& event) override;
    void OnDisplaySearchListItem(wxCommandEvent& event) override;
    void OnHome(wxCommandEvent& event) override;
    void OnHtmlLink(wxHtmlLinkEvent& event) override;
    void OnIndexTextChange(wxCommandEvent& event) override;
    void OnIndexTextEnter(wxCommandEvent& event) override;
    void OnInfo(wxCommandEvent& event) override;
    void OnOpenArchive(wxCommandEvent& event) override;
    void OnPageChanged(wxBookCtrlEvent& event) override;
    void OnSearchCancel(wxCommandEvent& event) override;
    void OnSearchTextChanged(wxCommandEvent& event) override;
    void OnTextKeyDown(wxKeyEvent& event) override;

private:
    // Display an archive page (e.g. "wxTextCtrl.md"). Injects an inheritance
    // graph after the first <h1> when available.
    void DisplayArchivePage(const std::string& archive_name);

    // Render an inheritance graph SVG for class_name. Returns an HTML <img>
    // block, or an empty string when there is nothing to draw.
    std::string BuildInheritanceImage(const std::string& class_name);

    // Lazy-load index items from archive_file on first tab visit.
    void PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
                              wxTextCtrl* filter_ctrl, const wxString& default_filter,
                              std::vector<std::string>& item_store);

    // Apply filter_text to listbox (incremental filter on the full item list).
    void ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
                     const wxString& filter_text);

    // Return the listbox paired with the textctrl that fired the event, or nullptr.
    wxListBox* GetActiveIndexListbox(const wxObject* source) const;

    // Show the find-in-page dialog.
    void OnFind(wxCommandEvent& event);

    bool m_archive_open { false };

    // Index item stores (full unfiltered lists kept for incremental filtering)
    std::vector<std::string> m_classes_items;
    std::vector<std::string> m_events_items;
    std::vector<std::string> m_functions_items;
    std::vector<std::string> m_overviews_items;

    // Inheritance graph support
    std::unordered_map<std::string, InheritEntry> m_inherit_map;
    std::string m_current_graph_fs_name;  // memory-FS name of the live graph image
    int m_graph_counter { 0 };

    // Toolbar tool ID for the Find button (assigned dynamically)
    int m_find_tool_id { wxID_NONE };
};
