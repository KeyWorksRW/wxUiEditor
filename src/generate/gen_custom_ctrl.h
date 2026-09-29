//////////////////////////////////////////////////////////////////////////
// Purpose:   Custom Control generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-29-2026]

#pragma once

#include <set>
#include <string>

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// GenLang is defined in the force-included precompiled header (pch.h). It is declared here so
// this header remains self-contained when the precompiled header is not force-included.
enum class GenLang : unsigned int;

class CustomControlGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node* node, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};
