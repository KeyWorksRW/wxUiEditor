/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// Note that the Mockup code does not call Create() for forms.

class FrameFormGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;
    bool AfterChildrenCode(Code& code) override;
    bool HeaderCode(Code& code) override;
    bool BaseClassNameCode(Code& code) override;

    bool AllowPropertyChange(wxPropertyGridEvent* event, NodeProperty* prop, Node* node) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    wxue::string GetPythonHelpText(Node*) override { return "wx.Frame"; }
    wxue::string GetPythonURL(Node*) override { return "wx.Frame.html"; }

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;
};
