/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxDocParentFrame generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

// Note that the Mockup code does not call Create() for forms.

class DocParentFrameGenerator : public BaseGenerator
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

    // Note that XRC does not support this class

    tt_string GetPythonHelpText(Node*) override { return "wx.docview.DocParentFrame"; }
    tt_string GetPythonURL(Node*) override { return "wx.lib.docview.DocParentFrame.html"; }

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;
};

class DocChildFrame : public BaseGenerator
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

    // Note that XRC does not support this class

    tt_string GetPythonHelpText(Node*) override { return "wx.docview.DocParentFrame"; }
    tt_string GetPythonURL(Node*) override { return "wx.lib.docview.DocParentFrame.html"; }

    bool GetImports(Node*, std::set<std::string>& set_imports, GenLang language) override;
};
