/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable doc viewer panel (wxPanel + wxHtmlWindow + index tabs)
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <filesystem>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

#include <wx/panel.h>

class wxBookCtrlEvent;
class wxChoicebook;
class wxCommandEvent;
class wxHtmlLinkEvent;
class wxHtmlWindow;
class wxListBox;
class wxObject;
class wxTextCtrl;

// Direct base and derived classes for one class, loaded from data/inheritance.json.
struct InheritEntry
{
    std::vector<std::string> bases;
    std::vector<std::string> derived;
};

// Reusable documentation viewer panel.
//
// Owns a wxHtmlWindow and choicebook index tabs (classes/events/functions/overviews).
// Delegates all content operations to the wxueArchive singleton (ArchiveHandler).
// Has NO dependency on MainFrame or any wxUiEditor-specific types.
class DocViewPane : public wxPanel
{
public:
    DocViewPane() = default;
    ~DocViewPane() override = default;

    DocViewPane(const DocViewPane&) = delete;
    DocViewPane& operator=(const DocViewPane&) = delete;
    DocViewPane(DocViewPane&&) = delete;
    DocViewPane& operator=(DocViewPane&&) = delete;

    // Create the panel as a child of parent. Returns false on failure.
    bool Create(wxWindow* parent);

    // Open a documentation ZIP archive, register the SVG logo, load the search
    // index, populate the inheritance map, and display the home page.
    void OpenArchive(const std::filesystem::path& zip_path);

    // Display an archive page (e.g. "wxTextCtrl.md"). Injects an inheritance
    // graph after the first <h1> when available, then invokes m_on_status_update.
    void DisplayPage(const std::string& archive_name);

    // Navigate to the home page (index.md).
    void OnHome();

    // Open a modal find-in-page dialog and navigate to the first match.
    void OnFind();

    // Callback invoked whenever the displayed page changes.  May be left empty.
    // Set before calling OpenArchive or DisplayPage.
    std::function<void(const std::string&)> m_on_status_update;

private:
    // Event handlers (bound via Bind in Create)
    void OnHtmlLink(wxHtmlLinkEvent& event);
    void OnPageChanged(wxBookCtrlEvent& event);
    void OnDblClickListBox(wxCommandEvent& event);
    void OnFilterTextChange(wxCommandEvent& event);
    void OnFilterTextEnter(wxCommandEvent& event);

    // Build the inheritance-graph HTML block for class_name. Registers the SVG
    // in wxMemoryFSHandler and returns an HTML <img> block, or "" if unavailable.
    std::string BuildInheritanceImage(const std::string& class_name);

    // Lazy-load index items from archive_file on first tab visit.
    void PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
                              wxTextCtrl* filter_ctrl, const wxString& default_filter,
                              std::vector<std::string>& item_store);

    // Apply filter_text to listbox (incremental filter on the full item list).
    void ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
                     const wxString& filter_text);

    // Return the (listbox, item-vector) pair matched to a textctrl event source.
    // Returns {nullptr, nullptr} when source is not a known filter control.
    std::pair<wxListBox*, std::vector<std::string>*> GetListboxForTextCtrl(wxObject* source);

    wxHtmlWindow* m_html_win { nullptr };
    wxChoicebook* m_choicebook { nullptr };

    // Index tab pages
    wxPanel* m_classes_page { nullptr };
    wxPanel* m_events_page { nullptr };
    wxPanel* m_functions_page { nullptr };
    wxPanel* m_overviews_page { nullptr };

    // Index listboxes
    wxListBox* m_classes_listbox { nullptr };
    wxListBox* m_events_listbox { nullptr };
    wxListBox* m_functions_listbox { nullptr };
    wxListBox* m_overviews_listbox { nullptr };

    // Filter text controls
    wxTextCtrl* m_classes_textctrl { nullptr };
    wxTextCtrl* m_events_textctrl { nullptr };
    wxTextCtrl* m_functions_textctrl { nullptr };
    wxTextCtrl* m_overviews_textctrl { nullptr };

    // Full (unfiltered) item lists — kept for incremental filtering
    std::vector<std::string> m_classes_items;
    std::vector<std::string> m_events_items;
    std::vector<std::string> m_functions_items;
    std::vector<std::string> m_overviews_items;

    // Inheritance graph support
    std::unordered_map<std::string, InheritEntry> m_inherit_map;
    std::string m_current_graph_fs_name;  // memory-FS name of the live graph image
    int m_graph_counter { 0 };            // makes each graph image URL unique

    bool m_svg_logo_registered { false };
};
