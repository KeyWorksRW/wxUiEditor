/////////////////////////////////////////////////////////////////////////////
// Purpose:   Application-wide options
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include <ttstr.h>

class AppOptions
{
public:
    void ReadConfig();
    void WriteConfig();

    bool get_SizersAllBorders() { return m_sizers_all_borders; }
    bool get_SizersExpand() { return m_sizers_always_expand; }
    bool get_ClassAccess() { return m_class_access; }

    void set_SizersAllBorders(bool setting) { m_sizers_all_borders = setting; }
    void set_SizersExpand(bool setting) { m_sizers_always_expand = setting; }
    void set_ClassAccess(bool setting) { m_class_access = setting; }

    wxString& get_SrcExtension() { return m_src_extension; }
    wxString& get_HdrExtension() { return m_hdr_extension; }
    wxString& get_MemberPrefix() { return m_member_prefix; }

    void set_SrcExtension(const wxString& str) { m_src_extension = str; }
    void set_HdrExtension(const wxString& str) { m_hdr_extension = str; }
    void set_MemberPrefix(const wxString& str) { m_member_prefix = str; }

#if defined(_DEBUG)
    bool get_FilterWarningMsgs() { return m_filter_warning_msgs; }
    void set_FilterWarningMsgs(bool setting) { m_filter_warning_msgs = setting; }

    wxString& get_ChmFile() { return m_chm_file; }
    void set_ChmFile(const wxString& str) { m_chm_file = str; }
#endif  // _DEBUG


private:
    bool m_sizers_all_borders;
    bool m_sizers_always_expand;

    bool m_class_access;  // controls default member access for new widgets

    wxString m_src_extension;
    wxString m_hdr_extension;
    wxString m_member_prefix {  };

#if defined(_DEBUG)
    bool m_filter_warning_msgs { false };  // controls display of warning messages in debug CMsgFrame window

    wxString m_chm_file;  // path to wxWidgets chm file
#endif  // _DEBUG


};

extern AppOptions g_AppOptions;

inline AppOptions& GetAppOptions() { return g_AppOptions; }
