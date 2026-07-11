/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDialog generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PropSheetDlgGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;
    bool AfterChildrenCode(Code& code) override;
    bool HeaderCode(Code& code) override;
    bool BaseClassNameCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) override;
    void RequiredHandlers(Node* node, std::set<std::string>& handlers) override;

    void GenEvent(Code& code, NodeEvent* event, const std::string& class_name) override;
};
