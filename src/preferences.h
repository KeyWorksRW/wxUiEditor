/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

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

    // Add borders around all new sizers
    bool is_SizersAllBorders() const { return m_sizers_all_borders; }

    // Add expand flag to all new sizers
    bool is_SizersExpand() const { return m_sizers_always_expand; }

    // Add "m_" prefix to all new non-local member variables
    bool is_VarPrefix() const { return m_var_prefix; }

    // Enable WakaTime support
    bool is_WakaTimeEnabled() const { return m_enable_wakatime; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_VarPrefix(bool setting) { m_var_prefix = setting; }
    void set_WakaTimeEnabled(bool setting) { m_enable_wakatime = setting; }

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
    bool m_var_prefix { true };  // true to use "m_" prefix for member variables
};
