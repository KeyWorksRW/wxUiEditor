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
    wxObject* CreateMockup([[maybe_unused]] Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;
    bool AfterChildrenCode(Code& code) override;

    bool HeaderCode(Code& code) override;
    bool BaseClassNameCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    std::optional<wxue::string> GetHint(NodeProperty* prop) override;
    bool PopupMenuAddCommands(NavPopupMenu* menu, Node* node) override;

    std::vector<Node*> GetChildPanes(Node* parent);

    int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) override;
    void RequiredHandlers(Node* node, std::set<std::string>& handlers) override;
};

class WizardPageGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup([[maybe_unused]] Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;

    bool PopupMenuAddCommands(NavPopupMenu* menu, Node* node) override;

    int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) override;
    void RequiredHandlers(Node* node, std::set<std::string>& handlers) override;
};
