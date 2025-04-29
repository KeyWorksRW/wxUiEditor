/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2024 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// Note that the Mockup code does not call Create() for forms.

class AuiMdiFrameGenerator : public BaseGenerator
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

    tt_string GetPythonHelpText(Node*) override { return "wx.Frame"; }
    tt_string GetPythonURL(Node*) override { return "wx.Frame.html"; }

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;
};
