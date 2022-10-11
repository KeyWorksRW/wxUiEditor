/////////////////////////////////////////////////////////////////////////////
// Purpose:   Main application class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>  // wxAppBase class and macros used for declaration of wxApp

#include "node_classes.h"  // Forward defintions of Node classes

class Project;

namespace pugi
{
    class xml_document;
}

class ImportXML;
class MainFrame;
class ProjectSettings;

struct EmbeddedImage;
struct ImageBundle;

constexpr const auto ImportProjectVersion = 13;

struct PREFS
{
    enum PREVIEW_TYPE
    {
        PREVIEW_TYPE_XRC = 0,
        PREVIEW_TYPE_BOTH,
        PREVIEW_TYPE_CPP,
    };

    void ReadConfig();
    void WriteConfig();

    bool is_SizersAllBorders() const { return m_sizers_all_borders; }
    bool is_SizersExpand() const { return m_sizers_always_expand; }
    bool is_WakaTimeEnabled() const { return m_enable_wakatime; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_isWakaTimeEnabled(bool setting) { m_enable_wakatime = setting; }

    long GetDebugFlags() const { return m_flags; }
    void SetDebugFlags(long flags) { m_flags = flags; }

    long GetProjectFlags() const { return m_project_flags; }
    void SetProjectFlags(long flags) { m_project_flags = flags; }

    PREVIEW_TYPE GetPreviewType() const { return m_preview_type; }
    void SetPreviewType(PREVIEW_TYPE type) { m_preview_type = type; }

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

private:
    // These store both Debug and INTERNAL flags
    long m_flags { 0 };

    long m_project_flags { 0 };
    PREVIEW_TYPE m_preview_type { PREVIEW_TYPE_XRC };

    bool m_sizers_all_borders { true };
    bool m_sizers_always_expand { true };
    bool m_enable_wakatime { true };
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

    MainFrame* GetMainFrame() { return m_frame; }

    // Only call this if you need the reference count incremented.
    const ProjectSharedPtr& GetProjectPtr() { return m_project; };

    Project* GetProject() { return m_project.get(); };

    PREFS& Preferences() { return m_prefs; }

    bool isFireCreationMsgs() const { return (m_prefs.GetDebugFlags() & PREFS::PREFS_CREATION_MSG); }

    bool IsPjtMemberPrefix() const { return (m_prefs.GetProjectFlags() & PREFS::PREFS_PJT_MEMBER_PREFIX); }

    void SetLanguage(wxLanguage language) { m_lang = language; }
    wxLanguage GetLanguage() { return m_lang; }

#if defined(_DEBUG) || defined(INTERNAL_TESTING)

    void ShowMsgWindow();
    bool AutoMsgWindow() const
    {
        return (m_prefs.GetDebugFlags() & PREFS::PREFS_MSG_WINDOW);
    }

    void DbgCurrentTest(wxCommandEvent& event);
#endif

    void SetMainFrameClosing()
    {
        m_isMainFrameClosing = true;
    }
    bool isMainFrameClosing()
    {
        return m_isMainFrameClosing;
    }

    void ShowPreferences(wxWindow* parent);

    auto GetProjectVersion()
    {
        return m_ProjectVersion;
    }

    bool AskedAboutMissingDir(const wxString path)
    {
        return (m_missing_dirs.find(path) != m_missing_dirs.end());
    }
    void AddMissingDir(const wxString path)
    {
        m_missing_dirs.insert(path);
    }

protected:
    bool OnInit() override;
    bool Import(ImportXML& import, ttString& file, bool append = false);

#if wxUSE_ON_FATAL_EXCEPTION && wxUSE_STACKWALKER
    void OnFatalException() override;
#endif

    int OnRun() override;
    int OnExit() override;

    auto LoadProject(pugi::xml_document& doc) -> std::shared_ptr<Project>;

private:
    // This is a shared_ptr because it can be put on the undo stack, and it can be selected
    // while a new project is being loaded.

    std::shared_ptr<Project> m_project;

    // Every time we try to write to a directory that doesn't exist, we ask the user if they
    // want to create it. If they choose No then we store the path here and never ask again
    // for the current session.
    std::set<wxString> m_missing_dirs;

    // ProjectSettings* m_pjtSettings { nullptr };

    MainFrame* m_frame { nullptr };

    wxLanguage m_lang;  // language specified by user
    wxLocale m_locale;  // locale we'll be using

    PREFS m_prefs;

    int m_ProjectVersion;
    bool m_isMainFrameClosing { false };
    bool m_isProject_updated { false };
};

DECLARE_APP(App)
