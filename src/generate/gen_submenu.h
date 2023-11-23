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
    bool ConstructionCode(Code&) override;
    bool AfterChildrenCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    tt_string GetPythonHelpText(Node*) override { return "wx.Menu.AppendSubMenu"; }
    tt_string GetPythonURL(Node*) override { return "wx.Menu.html#wx.Menu.AppendSubMenu"; }
};
