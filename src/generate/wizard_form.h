/////////////////////////////////////////////////////////////////////////////
// Purpose:   Wizard form class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class WizardFormGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* /*node*/, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    std::optional<ttlib::cstr> GenAdditionalCode(GenEnum::GenCodeType cmd, Node* node) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    std::vector<Node*> GetChildPanes(Node* parent);
};

class WizardPageGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* /*node*/, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
};
