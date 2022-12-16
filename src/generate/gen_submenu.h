/////////////////////////////////////////////////////////////////////////////
// Purpose:   SubMenu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class SubMenuGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::sview> CommonConstruction(Code& code) override;
    std::optional<ttlib::sview> CommonAdditionalCode(Code&, GenEnum::GenCodeType /* command */) override;

    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    ttlib::cstr GetPythonHelpText(Node*) override { return "wx.Menu.AppendSubMenu"; }
    ttlib::cstr GetPythonURL(Node*) override { return "wx.Menu.html#wx.Menu.AppendSubMenu"; }
};
