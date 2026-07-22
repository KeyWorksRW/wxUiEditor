/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel for displaying docs in wxWebView
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [07-06-2026]

#pragma once

#include "language_docs_base.h"

class CustomEvent;
class MainFrame;
class wxWebView;

enum class GenLang : unsigned int;

class LanguageDocs : public LanguageDocsBase
{
public:
    LanguageDocs(wxWindow* parent, MainFrame* frame);
    ~LanguageDocs() override;

    void ActivatePage();

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
    MainFrame* m_mainframe { nullptr };
    wxWebView* m_webview { nullptr };

    GenLang m_language { GenLang::cplusplus };
};
