/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPanel Form generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PanelFormGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;
    bool AfterChildrenCode(Code& code) override;
    bool HeaderCode(Code& code) override;
    bool BaseClassNameCode(Code& code) override;

    int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;
    void RequiredHandlers(Node* node, std::set<std::string>& handlers) override;

    wxue::string GetPythonHelpText(Node* node) override;
    wxue::string GetRubyHelpText(Node* node) override;
    wxue::string GetPythonURL(Node* node) override;
    wxue::string GetRubyURL(Node* node) override;
};
