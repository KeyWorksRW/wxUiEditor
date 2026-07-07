/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable doc viewer panel (wxHtmlWindow + index tabs)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-04-2026]

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

#include "doc_view_panel_base.h"

class wxListBox;
class wxObject;
class wxTextCtrl;

// Direct base and derived classes for one class, loaded from data/inheritance.json.
struct InheritEntry
{
    std::vector<std::string> bases;
    std::vector<std::string> derived;
};

class DocViewPanel : public DocViewPanelBase
{
public:
    DocViewPanel();  // If you use this constructor, you must call Create(parent) then InitPanel()
    DocViewPanel(wxWindow* parent);
    DocViewPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize, long style = wxTAB_TRAVERSAL,
                 const wxString& name = wxPanelNameStr)
    {
        Create(parent, id, pos, size, style, name);
        InitPanel();
    };

    // Open (or re-open) a documentation ZIP archive.
    // Returns false when the archive could not be opened.
    [[nodiscard]] bool OpenArchive(const wxString& zip_path);

    // True when an archive has been successfully opened.
    [[nodiscard]] bool IsArchiveOpen() const;

    // Navigate to the archive home page.
    void NavigateHome();

    // Run post-Create initialization: bind the Find toolbar button and Ctrl+F.
    // Called automatically by the constructors that invoke Create().
    void InitPanel();

protected:
    // Overrides for DocViewPanelBase virtual event handlers
    void OnDblClickListBox(wxCommandEvent& event) override;
    void OnDisplaySearchListItem(wxCommandEvent& event) override;
    void OnHome(wxCommandEvent& event) override;
    void OnHtmlLink(wxHtmlLinkEvent& event) override;
    void OnIndexTextChange(wxCommandEvent& event) override;
    void OnIndexTextEnter(wxCommandEvent& event) override;
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

    // Update the parent frame's status bar text.
    void SetStatusMessage(const wxString& msg);

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

    // Toolbar tool ID for the Find button (assigned dynamically in InitPanel)
    int m_find_tool_id { wxID_NONE };

    // Find-in-page state (persists across dialog invocations for "Find Next")
    std::string m_find_last_query;
    std::size_t m_find_last_pos { 0 };
};
