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

#include "generated/doc_view_panel_base.h"

class HtmlFindDlg;
class wxKeyEvent;
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
        if (!Create(parent, id, pos, size, style, name))
        {
            return;
        }
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
    void OnNavBack(wxCommandEvent& event) override;
    void OnNavForward(wxCommandEvent& event) override;
    void OnUpdateUI(wxUpdateUIEvent& event) override;
    void OnFind(wxCommandEvent& event) override;

    void SetStatusMessage(const wxString& msg);

    // Record a navigation event: push current page to back history, clear forward,
    // and set destination as the new current page. Must be called BEFORE the
    // actual page display.
    void RecordNavigation(const std::string& destination);

private:
    // Display an archive page (e.g. "wxTextCtrl.md"). Injects an inheritance
    // graph after the first <h1> when available.
    void DisplayArchivePage(const std::string& archive_name);

    // Render an inheritance graph SVG for class_name. Returns an HTML <img>
    // block, or an empty string when there is nothing to draw.
    std::string BuildInheritanceImage(const std::string& class_name);

    // Lazy-load index items from archive_file on first tab visit.
    static void PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
                                     wxTextCtrl* filter_ctrl, const wxString& default_filter,
                                     std::vector<std::string>& item_store);

    // Apply filter_text to listbox (incremental filter on the full item list).
    static void ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
                            const wxString& filter_text);

    // Return the listbox paired with the textctrl that fired the event, or nullptr.
    wxListBox* GetActiveIndexListbox(const wxObject* source) const;

    // Advance to the next find match (F3), wrapping to the start when the end is reached.
    void OnFindNext(wxKeyEvent& event);

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

    // Find-in-page dialog (modeless, created in InitPanel)
    HtmlFindDlg* m_find_dlg { nullptr };

    // Navigation history (browser-style back/forward)
    std::vector<std::string> m_back_history;
    std::vector<std::string> m_forward_history;
    std::string m_current_history_page;
    bool m_is_history_nav { false };
};
