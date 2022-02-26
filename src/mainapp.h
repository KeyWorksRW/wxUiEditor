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
class ImportXML;

// Current version of wxUiEditor project files
constexpr const auto curWxuiMajorVer = 1;
constexpr const auto curWxuiMinorVer = 4;
constexpr const auto curCombinedVer = 14;

constexpr const auto ImportProjectVersion = 13;

enum class compiler_standard : size_t
{

    c11,
    c17,
    c20,

};

class App : public wxApp
{
public:
    App();

    void AppendCrafter(wxArrayString& files);
    void AppendFormBuilder(wxArrayString& files);
    void AppendGlade(wxArrayString& files);
    void AppendSmith(wxArrayString& files);
    void AppendWinRes(const ttlib::cstr& rc_file, std::vector<ttlib::cstr>& m_dialogs);
    void AppendXRC(wxArrayString& files);

    bool LoadProject(const ttString& file);
    bool NewProject(bool create_empty = false);

    // Determines which import method to use based on the filename's extension
    bool ImportProject(ttString& file);

    compiler_standard GetCompilerVersion();

    MainFrame* GetMainFrame() { return m_frame; }

    const NodeSharedPtr& GetProjectPtr() { return m_project; };
    Node* GetProject() { return m_project.get(); };

    const ttlib::cstr& getProjectFileName();
    const ttlib::cstr& getProjectPath();
    ttString GetProjectFileName();
    ttString GetProjectPath();
    ttString GetArtDirectory();
    ttString GetBaseDirectory();
    ttString GetDerivedDirectory();

    wxImage GetImage(const ttlib::cstr& description);
#if wxCHECK_VERSION(3, 1, 6)
    wxBitmapBundle GetImageBundle(const ttlib::cstr& description);
#else
    wxBitmap GetImageBundle(const ttlib::cstr& description);
#endif

    ProjectSettings* GetProjectSettings() { return m_pjtSettings; };

    // clang-format off
    enum : long
    {
        PREFS_MSG_WINDOW    = 1 << 2,   // automatically create CMsgFrame window
        PREFS_MSG_INFO      = 1 << 3,   // filter AddInfoMsg
        PREFS_MSG_EVENT     = 1 << 4,   // filter AddEventMsg
        PREFS_MSG_WARNING   = 1 << 5,   // filter AddWarningMsg

        PREFS_CREATION_MSG  = 1 << 6,  // Calls MSG_INFO when nav, prop, or mockup contents recreated
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

    bool isFireCreationMsgs() const noexcept { return (m_prefs.flags & PREFS_CREATION_MSG); }

    bool IsPjtMemberPrefix() const noexcept { return (m_prefs.project_flags & PREFS_PJT_MEMBER_PREFIX); }

    void SetLanguage(wxLanguage language) { m_lang = language; }
    wxLanguage GetLanguage() { return m_lang; }

#if defined(_DEBUG)

    void DbgCurrentTest(wxCommandEvent& event);
    void ShowMsgWindow();
    bool AutoMsgWindow() { return (m_prefs.flags & PREFS_MSG_WINDOW); }

#endif

    void SetMainFrameClosing() { m_isMainFrameClosing = true; }
    bool isMainFrameClosing() { return m_isMainFrameClosing; }

    void ShowPreferences(wxWindow* parent);

    auto GetProjectVersion() { return m_ProjectVersion; }

    bool AskedAboutMissingDir(const wxString path) { return (m_missing_dirs.find(path) != m_missing_dirs.end()); }
    void AddMissingDir(const wxString path) { m_missing_dirs.insert(path); }

protected:
    bool OnInit() override;
    bool Import(ImportXML& import, ttString& file, bool append = false);

#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
    void OnFatalException() override;
#endif

    int OnRun() override;
    int OnExit() override;

    auto LoadProject(pugi::xml_document& doc) -> std::shared_ptr<Node>;

private:
    std::shared_ptr<Node> m_project;

    // Every time we try to write to a directory that doesn't exist, we ask the user if they
    // want to create it. If they choose No then we store the path here and never ask again
    // for the current session.
    std::set<wxString> m_missing_dirs;

    ProjectSettings* m_pjtSettings { nullptr };

    uiPREFERENCES m_prefs;

    MainFrame* m_frame { nullptr };

    wxLanguage m_lang;  // language specified by user
    wxLocale m_locale;  // locale we'll be using

    int m_ProjectVersion;
    bool m_isMainFrameClosing { false };
    bool m_isProject_updated { false };
};

DECLARE_APP(App)
