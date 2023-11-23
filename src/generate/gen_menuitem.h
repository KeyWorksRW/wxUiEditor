/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Item  Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class MenuItemGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    void ChangeEnableState(wxPropertyGridManager*, NodeProperty*) override;
    bool modifyProperty(NodeProperty* prop, tt_string_view value) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};
