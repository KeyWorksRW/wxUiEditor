///////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel for displaying docs in wxWebView
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-06-2026]

#if wxUSE_WEBVIEW
    #include <wx/webview.h>
#endif

#include "language_docs.h"

#include "base_generator.h"   // BaseGenerator -- Base widget generator class
#include "cstm_event.h"       // CustomEvent -- Custom Event class
#include "mainframe.h"        // MainFrame -- Main window frame
#include "node.h"             // Node class
#include "project_handler.h"  // ProjectHandler -- Project file handler
#include "version.h"

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
    if (!m_webview)
    {
        const wxBusyCursor wait;

#if wxUSE_WEBVIEW
        m_webview = wxWebView::New(this, wxID_ANY, "about:blank");
        m_parent_sizer->Add(m_webview, wxSizerFlags(1).Expand().Border(wxALL));
#endif
        m_parent_sizer->Layout();
    }

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

void LanguageDocs::OnNodeSelected(CustomEvent& /* event unused */)
{
    if (!IsShown())
    {
        return;
    }

    ActivatePage();
}

void LanguageDocs::OnCPlus(wxCommandEvent& /* event */)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
        return;
    m_toolBar->ToggleTool(ID_CPLUS, true);
    m_toolBar->ToggleTool(ID_PYTHON, false);
    m_toolBar->ToggleTool(ID_RUBY, false);
    const wxBusyCursor wait;
    m_language = GenLang::cplusplus;
    if (auto* cur_sel = m_mainframe->getSelectedNode(); cur_sel)
    {
        if (auto* generator = cur_sel->get_Generator(); generator)
        {
            if (auto file = generator->GetHelpURL(cur_sel); !file.empty())
            {
                wxString url;
                url = (Project.get_LangVersion(GenLang::cplusplus) < CPP_WIDGETS_VERSION_3_3_0) ?
                          "https://docs.wxwidgets.org/3.2.8" :
                          "https://docs.wxwidgets.org/latest";

                if (!file.starts_with("group"))
                {
                    url += "/class";
                }
                url << file.wx();

                // Unfortunately, the wxWidgets documentation site now requires a captcha to verify
                // that the user is not a bot before allowing access to the documentation.
                // The website does not display at all in the IE embedded view that we use.

                // m_webview->LoadURL(url);

                m_webview->SetPage(
                    "<html><title>Displaying Documentation in Browser</title>"
                    "<body>"
                    "The C++ documentation is not accessible in the embedded browser because"
                    " the wxWidgets documentation site now requires a captcha to verify"
                    " that the user is not a bot before allowing access to the documentation."
                    " Instead, the documentation will be opened in the default web browser."
                    "</body></html>",
                    wxEmptyString);
                wxLaunchDefaultBrowser(url);
                return;
            }
        }
    }
    m_webview->SetPage("<html><title>Select Node</title>"
                       "<body>The selected node does not have any specific documentation for C++."
                       "</body></html>",
                       wxEmptyString);
#endif
}

void LanguageDocs::OnPython(wxCommandEvent& /* event */)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
        return;
    m_toolBar->ToggleTool(ID_CPLUS, false);
    m_toolBar->ToggleTool(ID_PYTHON, true);
    m_toolBar->ToggleTool(ID_RUBY, false);
    const wxBusyCursor wait;
    m_language = GenLang::python;
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
#if wxUSE_WEBVIEW
    if (!m_webview)
        return;
    m_toolBar->ToggleTool(ID_CPLUS, false);
    m_toolBar->ToggleTool(ID_PYTHON, false);
    m_toolBar->ToggleTool(ID_RUBY, true);
    const wxBusyCursor wait;
    m_language = GenLang::ruby;
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

void LanguageDocs::OnHome(wxCommandEvent& /* event unused */)
{
    ActivatePage();
}

void LanguageDocs::OnBack(wxCommandEvent& /* event unused */)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
        return;
    const wxBusyCursor wait;

    m_webview->GoBack();
#endif
}

void LanguageDocs::OnForward(wxCommandEvent& /* event unused */)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
        return;
    const wxBusyCursor wait;
    m_webview->GoForward();
#endif
}

void LanguageDocs::OnUpdateBack(wxUpdateUIEvent& event)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
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

void LanguageDocs::OnUpdateForward(wxUpdateUIEvent& event)
{
#if wxUSE_WEBVIEW
    if (!m_webview)
    {
        event.Enable(false);

        return;
    }

    event.Enable(m_webview->CanGoForward());
#endif
}
