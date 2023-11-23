/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTimer generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class TimerGenerator : public BaseGenerator
{
public:
    int GetRequiredVersion(Node* node) override;
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;
    void GenEvent(Code& code, NodeEvent* event, const std::string& class_name) override;

    static bool StartIfChildTimer(Node* form, Code& code);
};
