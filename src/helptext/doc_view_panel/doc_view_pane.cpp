/////////////////////////////////////////////////////////////////////////////
// Purpose:   Reusable doc viewer panel implementation
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "doc_view_panel/doc_view_pane.h"

#include <cstddef>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <wx/button.h>
#include <wx/choicebk.h>
#include <wx/dialog.h>
#include <wx/fs_mem.h>
#include <wx/html/htmlwin.h>
#include <wx/listbox.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/textctrl.h>

#include <glaze/glaze.hpp>

#include "archive_handler.h"
#include "doc_view_utils/find_in_page.h"
#include "inherit_graph.h"  // docparser::InheritGraphNode, RenderInheritanceSvg

// ---------------------------------------------------------------------------
//  Glaze meta — let glaze serialize/deserialize InheritEntry by name.
//  Placed here (not in the header) to avoid pulling glaze into every TU.
// ---------------------------------------------------------------------------

template <>
struct glz::meta<InheritEntry>
{
    using Type = InheritEntry;
    // NOLINTNEXTLINE(readability-avoid-auto) — type is detail::Object<glz::tuple<...>>, not
    // writable
    static constexpr auto value = glz::object("bases", &Type::bases, "derived", &Type::derived);
};

// ---------------------------------------------------------------------------
//  DocViewPane::Create — build the panel UI
// ---------------------------------------------------------------------------

bool DocViewPane::Create(wxWindow* parent)
{
    if (!wxPanel::Create(parent, wxID_ANY))
    {
        return false;
    }

    wxBoxSizer* const main_sizer = new wxBoxSizer(wxVERTICAL);

    wxSplitterWindow* const splitter =
        new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSP_LIVE_UPDATE);
    splitter->SetMinimumPaneSize(100);

    // ----- Left side: choicebook with index tabs -----

    m_choicebook = new wxChoicebook(splitter, wxID_ANY);

    // Build the Classes tab
    {
        m_classes_page = new wxPanel(m_choicebook);
        wxBoxSizer* const page_sizer = new wxBoxSizer(wxVERTICAL);
        m_classes_textctrl = new wxTextCtrl(m_classes_page, wxID_ANY, wxEmptyString,
                                            wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_classes_listbox = new wxListBox(m_classes_page, wxID_ANY, wxDefaultPosition,
                                          wxDefaultSize, 0, nullptr, wxLB_SINGLE);
        page_sizer->Add(m_classes_textctrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
        page_sizer->Add(m_classes_listbox, wxSizerFlags(1).Expand().Border(wxALL, 2));
        m_classes_page->SetSizer(page_sizer);
        m_choicebook->AddPage(m_classes_page, wxT("Classes"));
    }

    // Build the Events tab
    {
        m_events_page = new wxPanel(m_choicebook);
        wxBoxSizer* const page_sizer = new wxBoxSizer(wxVERTICAL);
        m_events_textctrl = new wxTextCtrl(m_events_page, wxID_ANY, wxEmptyString,
                                           wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_events_listbox = new wxListBox(m_events_page, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                                         0, nullptr, wxLB_SINGLE);
        page_sizer->Add(m_events_textctrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
        page_sizer->Add(m_events_listbox, wxSizerFlags(1).Expand().Border(wxALL, 2));
        m_events_page->SetSizer(page_sizer);
        m_choicebook->AddPage(m_events_page, wxT("Events"));
    }

    // Build the Functions tab
    {
        m_functions_page = new wxPanel(m_choicebook);
        wxBoxSizer* const page_sizer = new wxBoxSizer(wxVERTICAL);
        m_functions_textctrl = new wxTextCtrl(m_functions_page, wxID_ANY, wxEmptyString,
                                              wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_functions_listbox = new wxListBox(m_functions_page, wxID_ANY, wxDefaultPosition,
                                            wxDefaultSize, 0, nullptr, wxLB_SINGLE);
        page_sizer->Add(m_functions_textctrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
        page_sizer->Add(m_functions_listbox, wxSizerFlags(1).Expand().Border(wxALL, 2));
        m_functions_page->SetSizer(page_sizer);
        m_choicebook->AddPage(m_functions_page, wxT("Functions"));
    }

    // Build the Overviews tab
    {
        m_overviews_page = new wxPanel(m_choicebook);
        wxBoxSizer* const page_sizer = new wxBoxSizer(wxVERTICAL);
        m_overviews_textctrl = new wxTextCtrl(m_overviews_page, wxID_ANY, wxEmptyString,
                                              wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
        m_overviews_listbox = new wxListBox(m_overviews_page, wxID_ANY, wxDefaultPosition,
                                            wxDefaultSize, 0, nullptr, wxLB_SINGLE);
        page_sizer->Add(m_overviews_textctrl, wxSizerFlags(0).Expand().Border(wxALL, 2));
        page_sizer->Add(m_overviews_listbox, wxSizerFlags(1).Expand().Border(wxALL, 2));
        m_overviews_page->SetSizer(page_sizer);
        m_choicebook->AddPage(m_overviews_page, wxT("Overviews"));
    }

    // ----- Right side: HTML window -----

    m_html_win =
        new wxHtmlWindow(splitter, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO);

    splitter->SplitVertically(m_choicebook, m_html_win, 250);
    main_sizer->Add(splitter, wxSizerFlags(1).Expand());
    SetSizer(main_sizer);

    // ----- Bind events -----

    m_html_win->Bind(wxEVT_HTML_LINK_CLICKED, &DocViewPane::OnHtmlLink, this);
    m_choicebook->Bind(wxEVT_BOOKCTRL_PAGE_CHANGED, &DocViewPane::OnPageChanged, this);

    for (wxListBox* listbox:
         { m_classes_listbox, m_events_listbox, m_functions_listbox, m_overviews_listbox })
    {
        listbox->Bind(wxEVT_LISTBOX_DCLICK, &DocViewPane::OnDblClickListBox, this);
    }

    for (wxTextCtrl* textctrl:
         { m_classes_textctrl, m_events_textctrl, m_functions_textctrl, m_overviews_textctrl })
    {
        textctrl->Bind(wxEVT_TEXT, &DocViewPane::OnFilterTextChange, this);
        textctrl->Bind(wxEVT_TEXT_ENTER, &DocViewPane::OnFilterTextEnter, this);
    }

    return true;
}

// ---------------------------------------------------------------------------
//  DocViewPane::OpenArchive
// ---------------------------------------------------------------------------

void DocViewPane::OpenArchive(const std::filesystem::path& zip_path)
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
        wxMessageBox(wxString::FromUTF8(open_result.error()), wxT("Error"), wxOK | wxICON_ERROR);
        return;
    }

    if (!m_svg_logo_registered)
    {
        m_svg_logo_registered = wxueArchive.ExtractAndRegisterSvgLogo();
    }

    wxueArchive.LoadSearchIndex();

    // Optional: load data/inheritance.json for on-the-fly inheritance graphs.
    // Older archives without this file are not an error — graphs are skipped.
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
    wxueArchive.DisplayHomePage(*m_html_win);

    if (m_on_status_update)
    {
        m_on_status_update(zip_path.filename().string());
    }
}

// ---------------------------------------------------------------------------
//  DocViewPane::DisplayPage
// ---------------------------------------------------------------------------

void DocViewPane::DisplayPage(const std::string& archive_name)
{
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
        constexpr std::size_t H1_CLOSE_LEN = 5;  // length of "</h1>"
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

    if (m_on_status_update)
    {
        m_on_status_update(archive_name);
    }
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnHome
// ---------------------------------------------------------------------------

void DocViewPane::OnHome()
{
    wxueArchive.DisplayHomePage(*m_html_win);
    if (m_on_status_update)
    {
        m_on_status_update("index.md");
    }
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnFind — modal find-in-page dialog
// ---------------------------------------------------------------------------

void DocViewPane::OnFind()
{
    const std::string& markdown = wxueArchive.GetCurrentMarkdown();
    if (markdown.empty())
    {
        return;
    }

    wxDialog find_dlg(this, wxID_ANY, wxT("Find in page"), wxDefaultPosition);
    wxBoxSizer* const find_sizer = new wxBoxSizer(wxVERTICAL);

    wxTextCtrl* const text_ctrl = new wxTextCtrl(
        &find_dlg, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(300, -1), wxTE_PROCESS_ENTER);
    find_sizer->Add(text_ctrl, wxSizerFlags().Expand().Border(wxALL, 8));

    wxBoxSizer* const btn_sizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton* const find_next_btn = new wxButton(&find_dlg, wxID_FORWARD, wxT("&Find Next"));
    wxButton* const cancel_btn = new wxButton(&find_dlg, wxID_CANCEL, wxT("&Close"));
    btn_sizer->Add(find_next_btn, wxSizerFlags().Border(wxRIGHT, 8));
    btn_sizer->Add(cancel_btn, wxSizerFlags());
    find_sizer->Add(btn_sizer, wxSizerFlags().Center().Border(wxBOTTOM, 8));

    find_dlg.SetSizerAndFit(find_sizer);

    wxString search_text;
    bool search_requested = false;

    text_ctrl->Bind(wxEVT_TEXT_ENTER,
                    [&]([[maybe_unused]] wxCommandEvent& event_arg)
                    {
                        search_text = text_ctrl->GetValue();
                        search_requested = true;
                        find_dlg.EndModal(wxID_OK);
                    });

    find_next_btn->Bind(wxEVT_BUTTON,
                        [&]([[maybe_unused]] wxCommandEvent& event_arg)
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

    const std::size_t found_pos = FindInMarkdown(markdown, query);
    if (found_pos == std::string::npos)
    {
        return;
    }

    // Extract heading IDs from the cached HTML to locate the containing section.
    std::vector<std::pair<std::string, std::string>> heading_ids;
    std::ignore = AddHeadingIds(wxueArchive.GetCurrentHtml(), &heading_ids);

    const std::string section_id = FindSectionForMarkdownPos(markdown, found_pos, heading_ids);
    if (section_id.empty())
    {
        return;
    }

    const wxString anchor_href = wxString::FromUTF8("#" + section_id);
    std::ignore = m_html_win->LoadPage(anchor_href);
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnHtmlLink — delegate to ArchiveHandler
// ---------------------------------------------------------------------------

void DocViewPane::OnHtmlLink(wxHtmlLinkEvent& event)
{
    wxueArchive.OnHtmlLink(event.GetLinkInfo(), *m_html_win);

    const std::string& current_page = wxueArchive.GetCurrentPage();
    if (!current_page.empty() && m_on_status_update)
    {
        m_on_status_update(current_page);
    }
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnPageChanged — lazy-load index on first tab visit
// ---------------------------------------------------------------------------

void DocViewPane::OnPageChanged(wxBookCtrlEvent& event)
{
    const int page_sel = event.GetSelection();
    if (page_sel == wxNOT_FOUND)
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
        event.Skip();
        return;
    }

    if (wxueArchive.is_open() && listbox->IsEmpty())
    {
        PopulateIndexListbox(index_file, listbox, textctrl, default_filter, *item_store);
    }
    event.Skip();
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnDblClickListBox
// ---------------------------------------------------------------------------

void DocViewPane::OnDblClickListBox(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.utf8_string() + ".md";
    DisplayPage(archive_name);
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnFilterTextChange — incremental filter on text change
// ---------------------------------------------------------------------------

void DocViewPane::OnFilterTextChange(wxCommandEvent& event)
{
    const auto [listbox, item_vec] = GetListboxForTextCtrl(event.GetEventObject());
    if (listbox == nullptr || item_vec == nullptr)
    {
        event.Skip();
        return;
    }

    ApplyFilter(listbox, *item_vec, event.GetString());
}

// ---------------------------------------------------------------------------
//  DocViewPane::OnFilterTextEnter — select single match on Enter
// ---------------------------------------------------------------------------

void DocViewPane::OnFilterTextEnter(wxCommandEvent& event)
{
    wxListBox* const listbox = GetListboxForTextCtrl(event.GetEventObject()).first;
    if (listbox == nullptr)
    {
        event.Skip();
        return;
    }

    if (listbox->GetCount() == 1)
    {
        const std::string archive_name = listbox->GetString(0).utf8_string() + ".md";
        DisplayPage(archive_name);
    }
}

// ---------------------------------------------------------------------------
//  DocViewPane::BuildInheritanceImage
// ---------------------------------------------------------------------------

std::string DocViewPane::BuildInheritanceImage(const std::string& class_name)
{
    const std::unordered_map<std::string, InheritEntry>::const_iterator found =
        m_inherit_map.find(class_name);
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
        self_node.bases = entry.bases;  // edges: each base → this class
        nodes.push_back(std::move(self_node));
    }

    const std::size_t shown = std::min(entry.derived.size(), MAX_CHILDREN);
    for (std::size_t idx = 0; idx < shown; ++idx)
    {
        docparser::InheritGraphNode child_node;
        child_node.name = entry.derived[idx];
        child_node.url = entry.derived[idx] + ".md";
        child_node.bases = { class_name };  // edge: this class → child
        nodes.push_back(std::move(child_node));
    }

    if (entry.derived.size() > shown)
    {
        docparser::InheritGraphNode more_node;
        more_node.name = "(+" + std::to_string(entry.derived.size() - shown) + " more)";
        more_node.bases = { class_name };
        nodes.push_back(std::move(more_node));
    }

    // Use horizontal layout when many children to avoid an excessively wide row.
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

// ---------------------------------------------------------------------------
//  DocViewPane::PopulateIndexListbox
// ---------------------------------------------------------------------------

void DocViewPane::PopulateIndexListbox(const std::string& index_file, wxListBox* listbox,
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

    // Set filter text (may or may not fire wxEVT_TEXT — always apply explicitly).
    filter_ctrl->SetValue(default_filter);
    ApplyFilter(listbox, item_store, default_filter);
}

// ---------------------------------------------------------------------------
//  DocViewPane::ApplyFilter
// ---------------------------------------------------------------------------

void DocViewPane::ApplyFilter(wxListBox* listbox, const std::vector<std::string>& all_items,
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

// ---------------------------------------------------------------------------
//  DocViewPane::GetListboxForTextCtrl
// ---------------------------------------------------------------------------

std::pair<wxListBox*, std::vector<std::string>*>
    DocViewPane::GetListboxForTextCtrl(wxObject* source)
{
    if (source == m_classes_textctrl)
    {
        return { m_classes_listbox, &m_classes_items };
    }
    if (source == m_events_textctrl)
    {
        return { m_events_listbox, &m_events_items };
    }
    if (source == m_functions_textctrl)
    {
        return { m_functions_listbox, &m_functions_items };
    }
    if (source == m_overviews_textctrl)
    {
        return { m_overviews_listbox, &m_overviews_items };
    }
    return { nullptr, nullptr };
}
