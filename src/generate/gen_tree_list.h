/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxTreeCtrl component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class TreeListCtrlGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    void GenEvent(Code& code, NodeEvent* event, const std::string& class_name) override;

    std::pair<bool, tt_string> isLanguageVersionSupported(GenLang language) override;
    std::optional<tt_string> GetWarning(Node* node, GenLang language) override;
};

class TreeListCtrlColumnGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
};
