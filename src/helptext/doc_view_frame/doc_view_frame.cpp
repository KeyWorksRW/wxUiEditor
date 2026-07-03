/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame wrapper for DocViewPane — menu bar, status bar, modes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <sstream>
#include <string>

#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/filedlg.h>
#include <wx/fs_mem.h>
#include <wx/listbox.h>
#include <wx/sizer.h>
#include <wx/textctrl.h>

#include <glaze/glaze.hpp>

#include "doc_view_frame/doc_view_frame.h"

#include "archive_handler.h"
#include "doc_view_utils/find_in_page.h"
#include "inherit_graph.h"  // docparser::InheritGraphNode, RenderInheritanceSvg

// ---------------------------------------------------------------------------
//  Glaze meta — serialize/deserialize InheritEntry by name
// ---------------------------------------------------------------------------

template <>
struct glz::meta<InheritEntry>
{
    using Type = InheritEntry;
    static constexpr auto value = glz::object("bases", &Type::bases, "derived", &Type::derived);
};

// ---------------------------------------------------------------------------
//  Constructor
// ---------------------------------------------------------------------------

DocViewFrame::DocViewFrame(wxWindow* parent, const std::filesystem::path& zip_path) :
    DocViewFrame_base(parent)
{
    // Find the Find toolbar button by excluding the stored tool references
    for (std::size_t i = 0; i < m_aui_tool_bar->GetToolCount(); ++i)
    {
        const wxAuiToolBarItem* const tool = m_aui_tool_bar->FindToolByIndex(i);
        if (tool)
        {
            const int id = tool->GetId();
            if (id != m_home->GetId() && id != m_back->GetId() && id != m_forward->GetId())
            {
                m_find_tool_id = id;
                break;
            }
        }
    }
    if (m_find_tool_id != wxID_NONE)
    {
        Bind(wxEVT_TOOL, &DocViewFrame::OnFind, this, m_find_tool_id);
    }

    // Ctrl+F via CHAR_HOOK — more reliable than an accelerator table with
    // controls that capture keystrokes at a low level (wxHtmlWindow, etc.)
    Bind(wxEVT_CHAR_HOOK,
         [this](wxKeyEvent& key_event)
         {
             if (key_event.ControlDown() && key_event.GetKeyCode() == static_cast<int>('F'))
             {
                 wxCommandEvent dummy;
                 OnFind(dummy);
                 return;
             }
             key_event.Skip();
         });

    if (!zip_path.empty())
    {
        std::ignore = OpenArchive(zip_path);
    }
}

// ---------------------------------------------------------------------------
//  OpenArchive
// ---------------------------------------------------------------------------

bool DocViewFrame::OpenArchive(const std::filesystem::path& zip_path)
{
    // Reset state for the new archive
    m_inherit_map.clear();
    m_current_graph_fs_name.clear();

    m_classes_items.clear();
    m_events_items.clear();
    m_functions_items.clear();
    m_overviews_items.clear();

    m_classes_listbox->Clear();
    m_events_listbox->Clear();
    m_functions_listbox->Clear();
    m_overviews_listbox->Clear();

    std::expected<void, std::string> open_result = wxueArchive.OpenArchive(zip_path);
    if (!open_result)
    {
        SetStatusText(wxString::FromUTF8(open_result.error()));
        return false;
    }

    // Register SVG logo in the memory file system
    std::ignore = wxueArchive.ExtractAndRegisterSvgLogo();

    // Load the full-text search index
    std::ignore = wxueArchive.LoadSearchIndex();

    // Optional: load data/inheritance.json for on-the-fly inheritance graphs.
    if (const std::expected<std::string, std::string> inherit_result =
            wxueArchive.ReadFile("data/inheritance.json");
        inherit_result)
    {
        std::unordered_map<std::string, InheritEntry> parsed;
        const glz::error_ctx parse_err = glz::read_json(parsed, *inherit_result);
        if (!parse_err)
        {
            m_inherit_map = std::move(parsed);
        }
    }

    // Populate the classes tab immediately (default tab shown on startup).
    PopulateIndexListbox("data/classes.md", m_classes_listbox, m_classes_textctrl, wxT("wx"),
                         m_classes_items);

    // Display home page
    std::ignore = wxueArchive.DisplayHomePage(*m_html_win);

    m_archive_open = true;

    // Update the title bar to reflect the loaded archive.
    const wxString archive_name = wxString::FromUTF8(zip_path.filename().string());
    SetTitle(wxString::Format("Doc Viewer - %s", archive_name));
    SetStatusText(wxString::Format("Loaded %s", archive_name));

    return true;
}

bool DocViewFrame::IsArchiveOpen() const
{
    return m_archive_open;
}

// ---------------------------------------------------------------------------
//  DisplayArchivePage
// ---------------------------------------------------------------------------

void DocViewFrame::DisplayArchivePage(const std::string& archive_name)
{
    if (!wxueArchive.is_open())
    {
        return;
    }

    if (!wxueArchive.DisplayArchivePage(archive_name, *m_html_win))
    {
        return;
    }

    // Inject inheritance graph after </h1> if data is available for this page.
    const std::string class_name = std::filesystem::path(archive_name).stem().string();
    const std::string img_block = BuildInheritanceImage(class_name);
    if (!img_block.empty())
    {
        std::string modified_html = wxueArchive.GetCurrentHtml();
        const std::size_t h1_end_pos = modified_html.find("</h1>");
        constexpr std::size_t H1_CLOSE_LEN = 5;
        if (h1_end_pos != std::string::npos)
        {
            modified_html.insert(h1_end_pos + H1_CLOSE_LEN, "\n" + img_block);
        }
        else
        {
            modified_html.insert(0, img_block);
        }
        m_html_win->SetPage(wxString::FromUTF8(modified_html));
    }

    SetStatusText(wxString::FromUTF8(archive_name));
}

// ---------------------------------------------------------------------------
//  Event handlers — file / navigation
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
    if (wxueArchive.is_open())
    {
        std::ignore = wxueArchive.DisplayHomePage(*m_html_win);
        SetStatusText("index.md");
    }
}

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

// ---------------------------------------------------------------------------
//  Event handlers — HTML window
// ---------------------------------------------------------------------------

void DocViewFrame::OnHtmlLink(wxHtmlLinkEvent& event)
{
    wxueArchive.OnHtmlLink(event.GetLinkInfo(), *m_html_win);

    const std::string& current_page = wxueArchive.GetCurrentPage();
    if (!current_page.empty())
    {
        SetStatusText(wxString::FromUTF8(current_page));
    }
}

// ---------------------------------------------------------------------------
//  Event handlers — index tabs (classes / events / functions / overviews)
// ---------------------------------------------------------------------------

void DocViewFrame::OnPageChanged(wxBookCtrlEvent& event)
{
    const int page_sel = event.GetSelection();
    if (page_sel == wxNOT_FOUND)
    {
        event.Skip();
        return;
    }

    if (!wxueArchive.is_open())
    {
        event.Skip();
        return;
    }

    const wxWindow* const changed_page = m_choicebook->GetPage(page_sel);

    std::string index_file;
    wxListBox* listbox = nullptr;
    wxTextCtrl* textctrl = nullptr;
    wxString default_filter;
    std::vector<std::string>* item_store = nullptr;

    if (changed_page == m_classes_page)
    {
        index_file = "data/classes.md";
        listbox = m_classes_listbox;
        textctrl = m_classes_textctrl;
        default_filter = wxT("wx");
        item_store = &m_classes_items;
    }
    else if (changed_page == m_events_page)
    {
        index_file = "data/events.md";
        listbox = m_events_listbox;
        textctrl = m_events_textctrl;
        default_filter = wxT("wx");
        item_store = &m_events_items;
    }
    else if (changed_page == m_functions_page)
    {
        index_file = "data/functions.md";
        listbox = m_functions_listbox;
        textctrl = m_functions_textctrl;
        item_store = &m_functions_items;
    }
    else if (changed_page == m_overviews_page)
    {
        index_file = "data/overviews.md";
        listbox = m_overviews_listbox;
        textctrl = m_overviews_textctrl;
        item_store = &m_overviews_items;
    }
    else
    {
        // Search page or unknown — nothing to lazy-load
        event.Skip();
        return;
    }

    if (listbox->IsEmpty())
    {
        PopulateIndexListbox(index_file, listbox, textctrl, default_filter, *item_store);
    }
    event.Skip();
}

void DocViewFrame::OnDblClickListBox(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.utf8_string() + ".md";
    DisplayArchivePage(archive_name);
}

void DocViewFrame::OnIndexTextChange(wxCommandEvent& event)
{
    wxListBox* const listbox = GetActiveIndexListbox(event.GetEventObject());
    if (listbox == nullptr)
    {
        event.Skip();
        return;
    }

    const std::vector<std::string>* item_vec = nullptr;
    if (listbox == m_classes_listbox)
        item_vec = &m_classes_items;
    else if (listbox == m_events_listbox)
        item_vec = &m_events_items;
    else if (listbox == m_functions_listbox)
        item_vec = &m_functions_items;
    else if (listbox == m_overviews_listbox)
        item_vec = &m_overviews_items;

    if (item_vec == nullptr)
    {
        event.Skip();
        return;
    }

    ApplyFilter(listbox, *item_vec, event.GetString());
}

void DocViewFrame::OnIndexTextEnter(wxCommandEvent& event)
{
    wxListBox* const listbox = GetActiveIndexListbox(event.GetEventObject());
    if (listbox == nullptr)
    {
        event.Skip();
        return;
    }

    if (listbox->GetCount() == 1)
    {
        const std::string archive_name = listbox->GetString(0).utf8_string() + ".md";
        DisplayArchivePage(archive_name);
    }
}

void DocViewFrame::OnTextKeyDown(wxKeyEvent& event)
{
    const int key_code = event.GetKeyCode();

    // Find which listbox is on the active choicebook page
    const int page_sel = m_choicebook->GetSelection();
    if (page_sel == wxNOT_FOUND)
    {
        event.Skip();
        return;
    }

    const wxWindow* const changed_page = m_choicebook->GetPage(page_sel);
    wxListBox* listbox = nullptr;

    if (changed_page == m_classes_page)
        listbox = m_classes_listbox;
    else if (changed_page == m_events_page)
        listbox = m_events_listbox;
    else if (changed_page == m_functions_page)
        listbox = m_functions_listbox;
    else if (changed_page == m_overviews_page)
        listbox = m_overviews_listbox;
    else if (changed_page == m_search_page)
        listbox = m_search_listbox;

    if (listbox == nullptr || listbox->GetCount() == 0)
    {
        event.Skip();
        return;
    }

    // Down arrow: move selection down, wrapping at bottom
    if (key_code == WXK_DOWN)
    {
        const int sel = listbox->GetSelection();
        if (sel == wxNOT_FOUND)
            listbox->SetSelection(0);
        else if (sel + 1 < static_cast<int>(listbox->GetCount()))
            listbox->SetSelection(sel + 1);
        else
            listbox->SetSelection(0);
        return;
    }

    // Up arrow: move selection up, wrapping at top
    if (key_code == WXK_UP)
    {
        const int sel = listbox->GetSelection();
        if (sel == wxNOT_FOUND)
            listbox->SetSelection(static_cast<int>(listbox->GetCount()) - 1);
        else if (sel > 0)
            listbox->SetSelection(sel - 1);
        else
            listbox->SetSelection(static_cast<int>(listbox->GetCount()) - 1);
        return;
    }

    // Enter: display the selected item
    if (key_code == WXK_RETURN)
    {
        const int sel = listbox->GetSelection();
        if (sel != wxNOT_FOUND)
        {
            const std::string archive_name = listbox->GetString(sel).utf8_string() + ".md";
            DisplayArchivePage(archive_name);
            return;
        }
    }

    event.Skip();
}

// ---------------------------------------------------------------------------
//  Event handlers — search
// ---------------------------------------------------------------------------

void DocViewFrame::OnSearchTextChanged(wxCommandEvent& event)
{
    const wxString query = event.GetString();
    m_search_listbox->Clear();

    if (query.empty() || !wxueArchive.HasSearchIndex())
    {
        return;
    }

    const std::string query_utf8 = query.utf8_string();
    auto results = wxueArchive.SearchIncremental(query_utf8);
    if (!results)
    {
        return;
    }

    for (const auto& result: *results)
    {
        const std::string archive_path = wxueArchive.GetArchivePathForDoc(result.doc_id);
        if (!archive_path.empty())
        {
            // Strip the .md extension for display
            std::string display_name = std::filesystem::path(archive_path).stem().string();
            m_search_listbox->Append(wxString::FromUTF8(display_name));
        }
    }
}

void DocViewFrame::OnSearchCancel([[maybe_unused]] wxCommandEvent& event)
{
    m_search_ctrl->Clear();
    m_search_listbox->Clear();
}

void DocViewFrame::OnDisplaySearchListItem(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.utf8_string() + ".md";
    DisplayArchivePage(archive_name);

    // Switch to the search page to show results context
    // (the page is already visible since we're on the search tab)
}

// ---------------------------------------------------------------------------
//  Event handlers — info
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
//  Find-in-page
// ---------------------------------------------------------------------------

void DocViewFrame::OnFind([[maybe_unused]] wxCommandEvent& event)
{
    if (!wxueArchive.is_open())
    {
        SetStatusText("No archive loaded");
        return;
    }

    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        SetStatusText("No page loaded");
        return;
    }

    // Create a simple modal find dialog
    wxDialog find_dlg(this, wxID_ANY, "Find in page", wxDefaultPosition);
    auto* find_sizer = new wxBoxSizer(wxVERTICAL);

    auto* text_ctrl = new wxTextCtrl(&find_dlg, wxID_ANY, wxEmptyString, wxDefaultPosition,
                                     wxSize(300, -1), wxTE_PROCESS_ENTER);
    find_sizer->Add(text_ctrl, wxSizerFlags().Expand().Border(wxALL, 8));

    auto* btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* find_next_btn = new wxButton(&find_dlg, wxID_FORWARD, "&Find Next");
    auto* cancel_btn = new wxButton(&find_dlg, wxID_CANCEL, "&Close");
    btn_sizer->Add(find_next_btn, wxSizerFlags().Border(wxRIGHT, 8));
    btn_sizer->Add(cancel_btn, wxSizerFlags());
    find_sizer->Add(btn_sizer, wxSizerFlags().Center().Border(wxBOTTOM, 8));

    find_dlg.SetSizerAndFit(find_sizer);

    wxString search_text;
    bool search_requested = false;

    text_ctrl->Bind(wxEVT_TEXT_ENTER,
                    [&](wxCommandEvent&)
                    {
                        search_text = text_ctrl->GetValue();
                        search_requested = true;
                        find_dlg.EndModal(wxID_OK);
                    });

    find_next_btn->Bind(wxEVT_BUTTON,
                        [&](wxCommandEvent&)
                        {
                            search_text = text_ctrl->GetValue();
                            search_requested = true;
                            find_dlg.EndModal(wxID_OK);
                        });

    find_dlg.ShowModal();

    if (!search_requested || search_text.empty())
    {
        return;
    }

    const std::string query = search_text.utf8_string();

    // Search the raw markdown for the query
    const std::size_t found_pos = FindInMarkdown(markdown, query);
    if (found_pos == std::string::npos)
    {
        SetStatusText(wxString::Format("Not found: %s", search_text));
        return;
    }

    // Extract heading IDs from the cached HTML
    std::vector<std::pair<std::string, std::string>> heading_ids;
    std::ignore = AddHeadingIds(wxueArchive.GetCurrentHtml(), &heading_ids);

    const std::string section_id = FindSectionForMarkdownPos(markdown, found_pos, heading_ids);

    if (section_id.empty())
    {
        SetStatusText(
            wxString::Format("Found \"%s\" but could not determine section", search_text));
        return;
    }

    // Navigate to that section
    const wxString anchor_href = wxString::FromUTF8("#" + section_id);
    const bool loaded = m_html_win->LoadPage(anchor_href);
    if (loaded)
    {
        SetStatusText(wxString::Format("Found \"%s\" in section #%s", search_text,
                                       wxString::FromUTF8(section_id)));
    }
    else
    {
        SetStatusText(
            wxString::Format("Found \"%s\" but section anchor failed to load", search_text));
    }
}

// ---------------------------------------------------------------------------
//  Private helpers
// ---------------------------------------------------------------------------

std::string DocViewFrame::BuildInheritanceImage(const std::string& class_name)
{
    const auto found = m_inherit_map.find(class_name);
    if (found == m_inherit_map.end())
    {
        return {};
    }
    const InheritEntry& entry = found->second;
    if (entry.bases.empty() && entry.derived.empty())
    {
        return {};
    }

    constexpr std::size_t MAX_CHILDREN = 24;
    std::vector<docparser::InheritGraphNode> nodes;
    nodes.reserve(entry.bases.size() + entry.derived.size() + 2);

    for (const std::string& base_class: entry.bases)
    {
        docparser::InheritGraphNode base_node;
        base_node.name = base_class;
        base_node.url = base_class + ".md";
        nodes.push_back(std::move(base_node));
    }

    {
        docparser::InheritGraphNode self_node;
        self_node.name = class_name;
        self_node.highlight = true;
        self_node.bases = entry.bases;
        nodes.push_back(std::move(self_node));
    }

    const std::size_t shown = std::min(entry.derived.size(), MAX_CHILDREN);
    for (std::size_t idx = 0; idx < shown; ++idx)
    {
        docparser::InheritGraphNode child_node;
        child_node.name = entry.derived[idx];
        child_node.url = entry.derived[idx] + ".md";
        child_node.bases = { class_name };
        nodes.push_back(std::move(child_node));
    }

    if (entry.derived.size() > shown)
    {
        docparser::InheritGraphNode more_node;
        more_node.name = "(+" + std::to_string(entry.derived.size() - shown) + " more)";
        more_node.bases = { class_name };
        nodes.push_back(std::move(more_node));
    }

    const bool use_horizontal = (entry.bases.size() <= 1 && entry.derived.size() > 8);

    double svg_width = 0.0;
    double svg_height = 0.0;
    const std::string svg =
        docparser::RenderInheritanceSvg(nodes, &svg_width, &svg_height, use_horizontal);
    if (svg.empty() || svg_width < 1.0 || svg_height < 1.0)
    {
        return {};
    }

    // Remove the previous graph image to avoid accumulating stale memory-FS files.
    if (!m_current_graph_fs_name.empty())
    {
        wxMemoryFSHandler::RemoveFile(wxString::FromUTF8(m_current_graph_fs_name));
    }
    ++m_graph_counter;
    m_current_graph_fs_name = "inherit_" + std::to_string(m_graph_counter) + ".svg";
    wxMemoryFSHandler::AddFileWithMimeType(wxString::FromUTF8(m_current_graph_fs_name), svg.data(),
                                           svg.size(), "image/svg+xml");

    return "<p><img src=\"memory:" + m_current_graph_fs_name + "\" alt=\"Inheritance graph for " +
           class_name + "\"></p>\n";
}

void DocViewFrame::PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
                                        wxTextCtrl* filter_ctrl, const wxString& default_filter,
                                        std::vector<std::string>& item_store)
{
    const std::expected<std::string, std::string> result = wxueArchive.ReadFile(index_file);
    if (!result)
    {
        return;
    }

    item_store.clear();
    std::istringstream stream(*result);
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        if (!line.empty())
        {
            item_store.push_back(line);
        }
    }

    filter_ctrl->SetValue(default_filter);
    ApplyFilter(listbox, item_store, default_filter);
}

void DocViewFrame::ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
                               const wxString& filter_text)
{
    const std::string filter = filter_text.utf8_string();

    listbox->Freeze();
    listbox->Clear();
    for (const std::string& item: all_items)
    {
        if (filter.empty() || item.contains(filter))
        {
            listbox->Append(wxString(item));
        }
    }
    listbox->Thaw();
}

wxListBox* DocViewFrame::GetActiveIndexListbox(const wxObject* source) const
{
    if (source == m_classes_textctrl)
    {
        return m_classes_listbox;
    }
    if (source == m_events_textctrl)
    {
        return m_events_listbox;
    }
    if (source == m_functions_textctrl)
    {
        return m_functions_listbox;
    }
    if (source == m_overviews_textctrl)
    {
        return m_overviews_listbox;
    }
    return nullptr;
}
