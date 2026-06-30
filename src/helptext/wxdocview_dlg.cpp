/////////////////////////////////////////////////////////////////////////////
// Purpose:   Display wxWidget documentation in a Dialog using wxHtmlWindow
// Author:    Ralph Walden
// Copyright: Copyright (c) 2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ..\..\LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <sstream>

#include "wxdocview_dlg.h"  // auto-generated: ../wxui/wxdocview_base.h and ../wxui/wxdocview_base.cpp

#include "archive_handler.h"  // Handles the wxUiEditorData.zip archive

#include "wx/msgdlg.h"

// Minimal stub until real SearchPanel is ported from wxViewer
class SearchPanel
{
public:
    SearchPanel() = default;
    ~SearchPanel() = default;
};

// If this constructor is used, the caller must call Create(parent)
wxDocView::wxDocView() {}

wxDocView::wxDocView(wxWindow* parent)
{
    Create(parent);
}

wxDocView::~wxDocView() = default;

static std::string_view zip_file = "C:/rwCode/wxLanguages/wxUiEditor/tests/wxWidgetsDocs.zip";

void wxDocView::OnInit(wxInitDialogEvent& /* event unused */)
{
    const std::filesystem::path zip_path = std::filesystem::path(zip_file);
    std::expected<void, std::string> open_result = wxueArchive.OpenArchive(zip_path);
    if (!open_result)
    {
        wxMessageBox(wxString::FromUTF8(open_result.error()), "Warning", wxOK | wxICON_WARNING);
        return;
    }

    m_svg_logo_registered = wxueArchive.ExtractAndRegisterSvgLogo();
}

void wxDocView::OnPageChanged(wxBookCtrlEvent& event)
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
    wxString default_filter = wxEmptyString;

    if (changed_page == m_classes_page)
    {
        index_file = "data/classes.md";
        listbox = m_classes_listbox;
        textctrl = m_classes_textctrl;
        default_filter = "wx";
    }
    else if (changed_page == m_events_page)
    {
        index_file = "data/events.md";
        listbox = m_events_listbox;
        textctrl = m_events_textctrl;
        default_filter = "wx";
    }
    else if (changed_page == m_functions_page)
    {
        index_file = "data/functions.md";
        listbox = m_functions_listbox;
        textctrl = m_functions_textctrl;
    }
    else if (changed_page == m_overviews_page)
    {
        index_file = "data/overviews.md";
        listbox = m_overviews_listbox;
        textctrl = m_overviews_textctrl;
    }
    else
    {
        event.Skip();
        return;
    }

    if (wxueArchive.is_open() && listbox->IsEmpty())
    {
        std::expected<std::string, std::string> result = wxueArchive.ReadFile(index_file);
        if (result)
        {
            std::istringstream stream(*result);
            std::string line;
            listbox->Freeze();
            while (std::getline(stream, line))
            {
                if (!line.empty() && line.back() == '\r')
                {
                    line.pop_back();
                }
                if (!line.empty())
                {
                    listbox->Append(wxString(line));
                }
            }
            listbox->Thaw();
            textctrl->SetValue(default_filter);
        }
    }
    event.Skip();
}

void wxDocView::OnSearchCancel(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnSearchCancel
}

void wxDocView::OnSearchTextChanged(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnSearchTextChanged
}

void wxDocView::OnDisplaySearchListItem(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnDisplaySearchListItem
}

void wxDocView::OnTextKeyDown(wxKeyEvent& /* event unused */)
{
    // TODO: Implement OnTextKeyDown
}

void wxDocView::OnIndexTextEnter(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnIndexTextEnter
}

void wxDocView::OnIndexTextChange(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnIndexTextChange
}

void wxDocView::OnDblClickListBox(wxCommandEvent& event)
{
    const wxString selection = event.GetString();
    if (selection.empty())
    {
        return;
    }

    const std::string archive_name = selection.ToStdString() + ".md";
    wxueArchive.DisplayArchivePage(archive_name, *m_html_win);
}

void wxDocView::OnHome(wxCommandEvent& /* event unused */)
{
    // TODO: Implement OnHome
}

void wxDocView::OnHtmlLink(wxHtmlLinkEvent& /* event unused */)
{
    // TODO: Implement OnHtmlLink
}
