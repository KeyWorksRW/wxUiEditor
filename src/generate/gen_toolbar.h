/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once  // NOLINT(#pragma once in main file)

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class ToolBarFormGenerator : public BaseGenerator
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

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

protected:
    void OnTool(wxCommandEvent& event);
};

class ToolBarGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /*wxparent*/, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

protected:
    void OnTool(wxCommandEvent& event);
};

class ToolGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override;
};

class ToolDropDownGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
};

class ToolSeparatorGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    tt_string GetHelpText(Node*) override;
    tt_string GetHelpURL(Node*) override;
    tt_string GetPythonHelpText(Node*) override;
    tt_string GetPythonURL(Node*) override;
    tt_string GetRubyHelpText(Node*) override;
    tt_string GetRubyURL(Node*) override;
};

class ToolStretchableGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
    tt_string GetHelpText(Node*) override;
    tt_string GetHelpURL(Node*) override;
    tt_string GetPythonHelpText(Node*) override;
    tt_string GetPythonURL(Node*) override;
    tt_string GetRubyHelpText(Node*) override;
    tt_string GetRubyURL(Node*) override;
};
