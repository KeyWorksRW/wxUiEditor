/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiToolBar generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// Note that there is no form version of wxAuiToolBar

class AuiToolBarFormGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    void GenEvent(Code&, NodeEvent*, const std::string&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node*, std::set<std::string>& /* set_imports */, GenLang language) override;

protected:
    void OnTool(wxCommandEvent& event);
};

class AuiToolBarGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* wxobject, wxWindow* /* wxparent */, Node* /* node */, bool /* is_preview */) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node*, std::set<std::string>& /* set_imports */, GenLang language) override;

protected:
    void OnTool(wxCommandEvent& event);
};

class AuiToolGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;
    bool GetIncludes(Node*, std::set<std::string>& /* set_src */, std::set<std::string>& /* set_hdr */,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override;
};

class AuiToolLabelGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
};

class AuiToolSpacerGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
};

class AuiToolStretchSpacerGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    int GetRequiredVersion(Node* /*node*/) override { return minRequiredVer + 1; }
};
