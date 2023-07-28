/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "preferences.h"

PREFS g_preferences;

void PREFS::ReadConfig()
{
    auto* config = wxConfig::Get();
    config->SetPath("/preferences");

    m_flags = config->ReadLong("flags", PREFS_MSG_WINDOW | PREFS_MSG_INFO | PREFS_MSG_EVENT | PREFS_MSG_WARNING);
    m_project_flags = config->ReadLong("project_flags", PREFS_PJT_MEMBER_PREFIX);
    m_preview_type = static_cast<PREVIEW_TYPE>(config->ReadLong("preview_type", PREVIEW_TYPE_XRC));

    m_sizers_all_borders = config->ReadBool("all_borders", true);
    m_sizers_always_expand = config->ReadBool("always_expand", true);
    m_var_prefix = config->ReadBool("var_prefix", true);

    m_enable_wakatime = config->ReadBool("enable_wakatime", true);
    m_dark_mode = config->ReadBool("dark_mode", false);
    m_is_load_last_project = config->ReadBool("load_last_project", false);
    m_is_right_propgrid = config->ReadBool("right_propgrid", false);
    m_is_cpp_snake_case = config->ReadBool("cpp_snake_case", false);

    m_cpp_widgets_version = config->Read("cpp_widgets_version", "3.2");

    m_colour_cpp = config->Read("cpp_colour", "#FF00FF");
    m_colour_python = config->Read("python_colour", "#FF00FF");
    m_colour_ruby = config->Read("ruby_colour", "#FF00FF");

    m_cpp_line_length = config->Read("cpp_line_length", 110);
    m_python_line_length = config->Read("python_line_length", 90);
    m_ruby_line_length = config->Read("ruby_line_length", 80);

    config->SetPath("/");
}

void PREFS::WriteConfig()
{
    auto* config = wxConfig::Get();
    config->SetPath("/preferences");

    config->Write("all_borders", m_sizers_all_borders);
    config->Write("always_expand", m_sizers_always_expand);
    config->Write("var_prefix", m_var_prefix);

    config->Write("enable_wakatime", m_enable_wakatime);
    config->Write("dark_mode", m_dark_mode);
    config->Write("load_last_project", m_is_load_last_project);
    config->Write("right_propgrid", m_is_right_propgrid);
    config->Write("cpp_snake_case", m_is_cpp_snake_case);

    config->Write("cpp_widgets_version", m_cpp_widgets_version.make_wxString());

    config->Write("cpp_colour", m_colour_cpp.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("python_colour", m_colour_python.GetAsString(wxC2S_HTML_SYNTAX));
    config->Write("ruby_colour", m_colour_ruby.GetAsString(wxC2S_HTML_SYNTAX));

    config->Write("cpp_line_length", m_cpp_line_length);
    config->Write("python_line_length", m_python_line_length);
    config->Write("ruby_line_length", m_ruby_line_length);

    config->SetPath("/");
}

PREFS& Preferences()
{
    return g_preferences;
}
