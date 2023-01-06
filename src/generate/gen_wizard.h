/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxWizard generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class WizardFormGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node*, wxObject* parent) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;

    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    bool GetIncludes(Node*, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    std::optional<ttlib::cstr> GetHint(NodeProperty*) override;
    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;

    std::vector<Node*> GetChildPanes(Node* parent);

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class WizardPageGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node*, wxObject* parent) override;

    bool ConstructionCode(Code&) override;

    bool PopupMenuAddCommands(NavPopupMenu*, Node*) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};
