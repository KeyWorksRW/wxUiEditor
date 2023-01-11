/////////////////////////////////////////////////////////////////////////////
// Purpose:   Set/Get wxUiEditor preferences
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>   // wxConfig base header

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
    m_enable_wakatime = config->ReadBool("enable_wakatime", true);
    m_var_prefix = config->ReadBool("var_prefix", true);

    config->SetPath("/");
}

void PREFS::WriteConfig()
{
    auto* config = wxConfig::Get();
    config->SetPath("/preferences");

    config->Write("all_borders", m_sizers_all_borders);
    config->Write("always_expand", m_sizers_always_expand);
    config->Write("enable_wakatime", m_enable_wakatime);
    config->Write("var_prefix", m_var_prefix);

    config->SetPath("/");
}

PREFS& Preferences()
{
    return g_preferences;
}
