/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ButtonGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;

    int GetRequiredVersion(Node* node) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;
};
