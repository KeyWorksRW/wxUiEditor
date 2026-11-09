/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel for displaying wxWidgets documentation for the current language
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-06-2026]

#pragma once

#include <string_view>

#include <wx/string.h>  // wxString

#include "language_docs_base.h"

class CustomEvent;
class DocViewPanel;
class MainFrame;
class wxWebView;

enum class GenLang : unsigned int;

class LanguageDocs : public LanguageDocsBase
{
public:
    LanguageDocs(wxWindow* parent, MainFrame* frame);
    ~LanguageDocs() override;

    void ActivatePage();

    // Display the documentation page for a wxWidgets class in the given language.
    // Returns false when the class has no documentation for that language.
    bool ShowClassDocs(std::string_view class_name, GenLang language);

protected:
    void OnNodeSelected(CustomEvent& event);

    void OnBack(wxCommandEvent& event) override;
    void OnCPlus(wxCommandEvent& event) override;
    void OnForward(wxCommandEvent& event) override;
    void OnHome(wxCommandEvent& event) override;
    void OnPython(wxCommandEvent& event) override;
    void OnRuby(wxCommandEvent& event) override;
    void OnUpdateBack(wxUpdateUIEvent& event) override;
    void OnUpdateForward(wxUpdateUIEvent& event) override;

private:
    // Create the child used to display documentation for the current language.
    void EnsureWebView();
    void EnsureDocViewPanel();

    // Show the child that belongs to m_language and hide the other one.
    void ShowWebViewChild();
    void ShowDocViewChild();

    // Navigate the doc panel to the page for the currently selected node.
    void DisplaySelectedNodeDocs();

    MainFrame* m_mainframe { nullptr };
    wxWebView* m_webview { nullptr };

    // Embedded wxWidgets documentation viewer, used for the C++ tab. The
    // wxWidgets documentation site blocks the embedded browser, so the C++
    // documentation is served from the archive embedded in the executable.
    DocViewPanel* m_doc_view_panel { nullptr };

    GenLang m_language { GenLang::cplusplus };
};

// Returns the web documentation URL for a wxWidgets class in a language that is
// documented on the web (Python, Ruby). Returns an empty wxString for C++ and for
// languages without web documentation.
wxString GetWxClassDocsURL(std::string_view class_name, GenLang language);
