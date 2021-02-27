/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

#include "node_classes.h"  // Forward defintions of Node classes

namespace pugi
{
    class xml_document;
}

class MainFrame;
class ProjectSettings;
class ImportInterface;

// Current version of wxUiEditor project files
constexpr const auto curWxuiMajorVer = 1;
constexpr const auto curWxuiMinorVer = 1;

class App : public wxApp
{
public:
    App();

    void ImportFormBuilder(wxArrayString& files);
    void ImportWinRes(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& m_dialogs);

    bool LoadProject(const ttString& file);
    bool NewProject();

    // Determines which import method to use based on the filename's extension
    bool ImportProject(ttString& file);

    MainFrame* GetMainFrame() { return m_frame; }

    const NodeSharedPtr& GetProjectPtr() { return m_project; };
    Node* GetProject() { return m_project.get(); };

    const ttlib::cstr& getProjectFileName();
    const ttlib::cstr& getProjectPath();
    ttString GetProjectFileName();
    ttString GetProjectPath();

    wxImage GetImage(ttlib::cstr filename);

    ProjectSettings* GetProjectSettings() { return m_pjtSettings; };

    // clang-format off
    enum
    {
        PREFS_MSG_WINDOW    = 1 << 2,   // automatically create CMsgFrame window
        PREFS_MSG_INFO      = 1 << 3,   // filter AddInfoMsg
        PREFS_MSG_EVENT     = 1 << 4,   // filter AddEventMsg
        PREFS_MSG_WARNING   = 1 << 5,   // filter AddWarningMsg
    };

    enum : long
    {
        PREFS_PJT_ALWAYS_LOCAL = 1 << 0,
        PREFS_PJT_MEMBER_PREFIX = 1 << 1,
    };

    // clang-format on

    struct uiPREFERENCES
    {
        long flags { 0 };  // BUGBUG: these need to be changed to debug-only flags

        long project_flags { 0 };

        wxString language;  // This is used in Debug builds for forcing a different UI
    };

    uiPREFERENCES& GetPrefs() { return m_prefs; }

    bool IsPjtMemberPrefix() const noexcept { return (m_prefs.project_flags & PREFS_PJT_MEMBER_PREFIX); }

    void SetLanguage(wxLanguage language) { m_lang = language; }
    wxLanguage GetLanguage() { return m_lang; }

#if defined(_DEBUG)

    void DbgCurrentTest(wxCommandEvent& event);
    void ShowMsgWindow();

#endif

    void SetMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    void ShowPreferences(wxWindow* parent);

protected:
    bool OnInit() override;
    bool Import(ImportInterface& import, ttString& file, bool append = false);

#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
    void OnFatalException() override;
#endif

    int OnRun() override;
    int OnExit() override;

    auto LoadProject(pugi::xml_document& doc) -> std::shared_ptr<Node>;

private:
    std::shared_ptr<Node> m_project;

    ProjectSettings* m_pjtSettings { nullptr };

    uiPREFERENCES m_prefs;

    MainFrame* m_frame { nullptr };

    wxLanguage m_lang;  // language specified by user
    wxLocale m_locale;  // locale we'll be using

    bool m_isMainFrameClosing { false };
    bool m_isProject_updated { false };
};

DECLARE_APP(App)
