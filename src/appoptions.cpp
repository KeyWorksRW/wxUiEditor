/////////////////////////////////////////////////////////////////////////////
// Purpose:   Application-wide options
// Author:    Ralph Walden
// Copyright: Copyright (c) 2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>  // wxConfig base header

#include "appoptions.h"

AppOptions g_AppOptions;

void AppOptions::ReadConfig()
{
    auto config = wxConfig::Get();
    config->SetPath("/options");

    m_sizers_all_borders = config->ReadBool("all_borders", true);
    m_sizers_always_expand = config->ReadBool("always_expand", true);
    m_enable_wakatime = config->ReadBool("enable_wakatime", true);

    config->SetPath("/");
}

void AppOptions::WriteConfig()
{
    auto config = wxConfig::Get();
    config->SetPath("/options");

    config->Write("all_borders", m_sizers_all_borders);
    config->Write("always_expand", m_sizers_always_expand);
    config->Write("enable_wakatime", m_enable_wakatime);

    config->SetPath("/");
}
