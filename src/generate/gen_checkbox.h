/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCheckBox generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class CheckBoxGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class Check3StateGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;

    tt_string GetPythonHelpText(Node*) override { return "wx.CheckBox"; }
    tt_string GetPythonURL(Node*) override { return "wx.CheckBox.html"; }
};
