/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class MenuGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;
    bool AfterChildrenCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    void ChangeEnableState(wxPropertyGridManager*, NodeProperty*) override;
    bool modifyProperty(NodeProperty* prop, tt_string_view value) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    tt_string GetPythonHelpText(Node*) override { return "wx.Menu"; }
    tt_string GetPythonURL(Node*) override { return "wx.Menu.html"; }
};
