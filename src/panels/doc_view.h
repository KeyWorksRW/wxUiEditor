/////////////////////////////////////////////////////////////////////////////
// Purpose:   Panel for displaying docs in wxWebView
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "docview_base.h"

class CustomEvent;
class wxWebView;
class MainFrame;

class DocViewPanel : public DocViewBase
{
public:
    DocViewPanel(wxWindow* parent, MainFrame* frame);

    void ActivatePage();

protected:
    void OnBack(wxCommandEvent& event) override;
    void OnForward(wxCommandEvent& event) override;
    void OnHome(wxCommandEvent& event) override;

    void OnUpdateBack(wxUpdateUIEvent& event) override;
    void OnUpdateForward(wxUpdateUIEvent& event) override;

    void OnCPlus(wxCommandEvent& event) override;
    void OnPython(wxCommandEvent& event) override;
    void OnRuby(wxCommandEvent& event) override;

    void OnNodeSelected(CustomEvent& event);

private:
    MainFrame* m_mainframe { nullptr };
    wxWebView* m_webview { nullptr };

    int m_language { GEN_LANG_CPLUSPLUS };
};
