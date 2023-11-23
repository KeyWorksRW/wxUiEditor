/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWebView generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class WebViewGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;

    void GenEvent(Code&, NodeEvent*, const std::string&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    std::optional<tt_string> GetWarning(Node* node, int language) override;
};
