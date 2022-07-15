/////////////////////////////////////////////////////////////////////////////
// Purpose:   Application-wide options
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

class AppOptions
{
public:
    void ReadConfig();
    void WriteConfig();

    bool get_SizersAllBorders() const { return m_sizers_all_borders; }
    bool get_SizersExpand() const { return m_sizers_always_expand; }
    bool get_isWakaTimeEnabled() const { return m_enable_wakatime; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_isWakaTimeEnabled(bool setting) { m_enable_wakatime = setting; }

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    bool get_FilterWarningMsgs()
    {
        return m_filter_warning_msgs;
    }
    void set_FilterWarningMsgs(bool setting)
    {
        m_filter_warning_msgs = setting;
    }

    wxString& get_ChmFile()
    {
        return m_chm_file;
    }
    void set_ChmFile(const wxString& str)
    {
        m_chm_file = str;
    }
#endif

private:
    bool m_sizers_all_borders;
    bool m_sizers_always_expand;
    bool m_enable_wakatime;

#if defined(_DEBUG) || defined(INTERNAL_TESTING)
    bool m_filter_warning_msgs { false };  // controls display of warning messages in debug CMsgFrame window

    wxString m_chm_file;  // path to wxWidgets chm file
#endif
};

extern AppOptions g_AppOptions;

inline AppOptions& GetAppOptions()
{
    return g_AppOptions;
}
