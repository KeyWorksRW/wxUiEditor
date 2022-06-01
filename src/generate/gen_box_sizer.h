/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxBoxSizer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class BoxSizerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* /*parent*/) override;
    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, bool /* add_comments */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};
