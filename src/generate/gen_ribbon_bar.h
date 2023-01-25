/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxRibbonBar -- form and regular
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxRibbonBarEvent;
class Code;
class Node;

class RibbonBarFormGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    void GenEvent(Code&, NodeEvent*, const std::string&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

protected:
    void OnPageChanged(wxRibbonBarEvent& event);
};

class RibbonBarGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

protected:
    void OnPageChanged(wxRibbonBarEvent& event);
};
