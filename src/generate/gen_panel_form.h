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

    bool CodeConstruction(Code&) override;
    bool CodeAdditionalCode(Code&, GenEnum::GenCodeType cmd) override;

    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};
