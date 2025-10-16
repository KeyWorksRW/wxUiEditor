/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStdDialogButtonSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class StdDialogButtonSizerGenerator : public BaseGenerator
{
public:
    auto CreateMockup(Node* node, wxObject* parent) -> wxObject* override;

    auto ConstructionCode(Code& code) -> bool override;

    auto GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) -> bool override;
    void GenEvent(Code& code, NodeEvent* event, const std::string& class_name) override;

    auto GenXrcObject(Node* node, pugi::xml_node& /* object */, size_t /* xrc_flags */)
        -> int override;
    void RequiredHandlers(Node* node, std::set<std::string>& /* handlers */) override;

    auto GetImports(Node* node, std::set<std::string>& set_imports, GenLang language)
        -> bool override;
};
