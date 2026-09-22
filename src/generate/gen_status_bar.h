//////////////////////////////////////////////////////////////////////////
// Purpose:   wxStatusBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-22-2026]

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class StatusBarGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;

    int GetRequiredVersion(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node* node, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers([[maybe_unused]] Node* node,
                          std::set<std::string>& /* handlers */) override;
};
