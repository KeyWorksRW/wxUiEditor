///////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel for displaying wxWidgets documentation for the current language
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-06-2026]

#include <string>
#include <string_view>

#if wxUSE_WEBVIEW
    #include <wx/webview.h>
#endif

#include "language_docs.h"

#include "base_generator.h"           // BaseGenerator -- Base widget generator class
#include "cstm_event.h"               // CustomEvent -- Custom Event class
#include "helptext/doc_view_frame.h"  // ResolveDocsZipPath
#include "helptext/doc_view_panel.h"  // DocViewPanel -- embedded wxWidgets docs viewer
#include "mainframe.h"                // MainFrame -- Main window frame
#include "node.h"                     // Node class
#include "project_handler.h"          // ProjectHandler -- Project file handler

LanguageDocs::LanguageDocs(wxWindow* parent, MainFrame* frame) : m_mainframe(frame)
{
    // Don't allow events to propagate any higher than this window.
    SetExtraStyle(wxWS_EX_BLOCK_EVENTS);

    std::ignore = Create(parent);

    frame->AddCustomEventHandler(GetEventHandler());

    Bind(EVT_NodeSelected, &LanguageDocs::OnNodeSelected, this);
}

LanguageDocs::~LanguageDocs()
{
    m_mainframe->RemoveCustomEventHandler(GetEventHandler());
}

void LanguageDocs::ActivatePage()
{
    // Language can change if there is a folder override
    m_language = Project.get_CodePreference(m_mainframe->getSelectedNode());

    m_toolBar->ToggleTool(ID_CPLUS, m_language == GenLang::cplusplus);
    m_toolBar->ToggleTool(ID_PYTHON, m_language == GenLang::python);
    m_toolBar->ToggleTool(ID_RUBY, m_language == GenLang::ruby);

    wxCommandEvent dummy;
    switch (m_language)
    {
        case GenLang::cplusplus:
            OnCPlus(dummy);
            break;
        case GenLang::python:
            OnPython(dummy);
            break;
        case GenLang::ruby:
            OnRuby(dummy);
            break;

        default:
            break;  // Currently, none of the other panels have documentation
    }
}

void LanguageDocs::OnNodeSelected([[maybe_unused]] CustomEvent& event)
{
    if (!IsShown())
    {
        return;
    }

    ActivatePage();
}

void LanguageDocs::OnCPlus(wxCommandEvent& /* event */)
{
    m_toolBar->ToggleTool(ID_CPLUS, true);
    m_toolBar->ToggleTool(ID_PYTHON, false);
    m_toolBar->ToggleTool(ID_RUBY, false);

    const wxBusyCursor wait;
    m_language = GenLang::cplusplus;

    ShowDocViewChild();
    DisplaySelectedNodeDocs();
}

void LanguageDocs::EnsureWebView()
{
#if wxUSE_WEBVIEW
    if (m_webview != nullptr)
    {
        return;
    }

    m_webview = wxWebView::New(this, wxID_ANY, "about:blank");
    m_parent_sizer->Add(m_webview, wxSizerFlags(1).Expand().Border(wxALL));
#endif
}

void LanguageDocs::EnsureDocViewPanel()
{
    if (m_doc_view_panel != nullptr)
    {
        return;
    }

    m_doc_view_panel =
        new DocViewPanel(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL);
    std::ignore = m_doc_view_panel->OpenArchive(ResolveDocsZipPath());
    m_parent_sizer->Add(m_doc_view_panel, wxSizerFlags(1).Expand().Border(wxALL));
}

void LanguageDocs::ShowWebViewChild()
{
    EnsureWebView();

    if (m_doc_view_panel != nullptr)
    {
        m_doc_view_panel->Hide();
    }
#if wxUSE_WEBVIEW
    if (m_webview != nullptr)
    {
        m_webview->Show();
    }
#endif
    m_parent_sizer->Layout();
}

void LanguageDocs::ShowDocViewChild()
{
    EnsureDocViewPanel();

#if wxUSE_WEBVIEW
    if (m_webview != nullptr)
    {
        m_webview->Hide();
    }
#endif
    m_doc_view_panel->Show();
    m_parent_sizer->Layout();
}

void LanguageDocs::DisplaySelectedNodeDocs()
{
    if (m_doc_view_panel == nullptr || !m_doc_view_panel->IsArchiveOpen())
    {
        return;
    }

    const Node* cur_sel = m_mainframe->getSelectedNode();
    if (cur_sel == nullptr)
    {
        return;
    }

    // Archived documentation pages are named after the wxWidgets class, e.g.
    // "wxTextCtrl.md". Nodes that are not wxWidgets classes keep the page that
    // is currently displayed.
    const std::string_view decl_name = cur_sel->get_DeclName();
    if (!decl_name.starts_with("wx"))
    {
        return;
    }

    std::string page_name(decl_name);
    page_name += ".md";
    std::ignore = m_doc_view_panel->DisplayPage(page_name);
}

void LanguageDocs::OnPython(wxCommandEvent& /* event */)
{
    m_toolBar->ToggleTool(ID_CPLUS, false);
    m_toolBar->ToggleTool(ID_PYTHON, true);
    m_toolBar->ToggleTool(ID_RUBY, false);
    const wxBusyCursor wait;
    m_language = GenLang::python;

    ShowWebViewChild();

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        return;
    }
    if (auto* cur_sel = m_mainframe->getSelectedNode(); cur_sel)
    {
        if (auto* generator = cur_sel->get_Generator(); generator)
        {
            if (auto file = generator->GetPythonURL(cur_sel); !file.empty())
            {
                wxString url("https://docs.wxpython.org/");
                url << file.wx();

                m_webview->LoadURL(url);
                return;
            }
        }
    }
    m_webview->SetPage("<html><title>Select Node</title>"
                       "<body>The selected node does not have any specific documentation for this "
                       "language.</body></html>",
                       wxEmptyString);
#endif
}

void LanguageDocs::OnRuby(wxCommandEvent& /* event */)
{
    m_toolBar->ToggleTool(ID_CPLUS, false);
    m_toolBar->ToggleTool(ID_PYTHON, false);
    m_toolBar->ToggleTool(ID_RUBY, true);
    const wxBusyCursor wait;
    m_language = GenLang::ruby;

    ShowWebViewChild();

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        return;
    }
    if (auto* cur_sel = m_mainframe->getSelectedNode(); cur_sel)
    {
        if (auto* generator = cur_sel->get_Generator(); generator)
        {
            if (auto file = generator->GetRubyURL(cur_sel); !file.empty())
            {
                wxString url("https://mcorino.github.io/wxRuby3/");
                url << file.wx();

                m_webview->LoadURL(url);
                return;
            }
        }
    }
    m_webview->SetPage("<html><title>Select Node</title>"
                       "<body>The selected node does not have any specific documentation for this "
                       "language.</body></html>",
                       wxEmptyString);
#endif
}

void LanguageDocs::OnHome([[maybe_unused]] wxCommandEvent& event)
{
    if (m_language == GenLang::cplusplus && m_doc_view_panel != nullptr)
    {
        m_doc_view_panel->NavigateHome();
        return;
    }

    ActivatePage();
}

void LanguageDocs::OnBack([[maybe_unused]] wxCommandEvent& event)
{
    if (m_language == GenLang::cplusplus && m_doc_view_panel != nullptr)
    {
        m_doc_view_panel->NavigateBack();
        return;
    }

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        return;
    }
    const wxBusyCursor wait;

    m_webview->GoBack();
#endif
}

void LanguageDocs::OnForward([[maybe_unused]] wxCommandEvent& event)
{
    if (m_language == GenLang::cplusplus && m_doc_view_panel != nullptr)
    {
        m_doc_view_panel->NavigateForward();
        return;
    }

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        return;
    }
    const wxBusyCursor wait;
    m_webview->GoForward();
#endif
}

void LanguageDocs::OnUpdateBack([[maybe_unused]] wxUpdateUIEvent& event)
{
    if (m_language == GenLang::cplusplus && m_doc_view_panel != nullptr)
    {
        event.Enable(m_doc_view_panel->CanNavigateBack());
        return;
    }

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        event.Enable(false);
        return;
    }

    if (auto history = m_webview->GetBackwardHistory(); history.size() == 1)
    {
        if (auto item = history[0]; item)
        {
            event.Enable(item->GetUrl() != "about:blank");
        }
        else
        {
            event.Enable(false);
        }
        return;
    }

    event.Enable(m_webview->CanGoBack());
#endif
}

void LanguageDocs::OnUpdateForward([[maybe_unused]] wxUpdateUIEvent& event)
{
    if (m_language == GenLang::cplusplus && m_doc_view_panel != nullptr)
    {
        event.Enable(m_doc_view_panel->CanNavigateForward());
        return;
    }

#if wxUSE_WEBVIEW
    if (m_webview == nullptr)
    {
        event.Enable(false);

        return;
    }

    event.Enable(m_webview->CanGoForward());
#endif
}
