/////////////////////////////////////////////////////////////////////////////
// Purpose:   Scroll window component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ScrolledCanvasGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;
    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    wxue::string GetHelpURL(Node*) override
    {
        return wxue::string("group__group__class__miscwnd.html");
    };
    std::optional<wxue::string> GetWarning(Node* node, GenLang language) override;
};

class ScrolledWindowGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    wxue::string GetHelpURL(Node*) override
    {
        return wxue::string("group__group__class__miscwnd.html");
    };
};
