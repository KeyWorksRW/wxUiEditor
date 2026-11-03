/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPropertyGridManager and wxPropertyGridPage generators
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PropertyGridManagerGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */,
                       bool /* is_preview */) override;
    bool AfterChildrenCode(Code&) override;

    bool ConstructionCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    bool GetImports(Node*, std::set<std::string>& /* set_imports */, GenLang language) override;

    int GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags) override;
    void RequiredHandlers(Node* node, std::set<std::string>& handlers) override;

protected:
    void OnPageChanged(wxPropertyGridEvent& event);
};

class PropertyGridPageGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
};
