/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonPage and wxRibbonPanel generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxRibbonBarEvent;
class Code;
class Node;

class RibbonPageGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};

class RibbonPanelGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};
