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
    m_class_access = config->ReadBool("class_access", true);

    config->Read("src_extension", &m_src_extension, ".cpp");
    config->Read("hdr_extension", &m_hdr_extension, ".h");
    config->Read("member_prefix", &m_member_prefix, "m_");
    config->SetPath("/");
}

void AppOptions::WriteConfig()
{
    auto config = wxConfig::Get();
    config->SetPath("/options");

    config->Write("all_borders", m_sizers_all_borders);
    config->Write("always_expand", m_sizers_always_expand);
    config->Write("class_access", m_class_access);

    config->Write("src_extension", m_src_extension);
    config->Write("hdr_extension", m_hdr_extension);
    config->Write("member_prefix", m_member_prefix);
    config->SetPath("/");
}
