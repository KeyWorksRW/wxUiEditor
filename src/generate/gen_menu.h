/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class MenuGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::sview> CommonConstruction(Code& code) override;
    std::optional<ttlib::sview> CommonAdditionalCode(Code& code, GenEnum::GenCodeType cmd) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    ttlib::cstr GetPythonHelpText(Node*) override { return "wx.Menu"; }
    ttlib::cstr GetPythonURL(Node*) override { return "wx.Menu.html"; }
};
