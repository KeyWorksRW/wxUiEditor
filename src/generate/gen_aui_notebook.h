//////////////////////////////////////////////////////////////////////////
// Purpose:   wxAuiNotebook generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class wxBookCtrlEvent;

class AuiNotebookGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    void AfterCreation(wxObject* /*wxobject*/, wxWindow* /*wxparent*/, Node* /* node */,
                       bool /* is_preview */) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node*, std::set<std::string>& /* set_imports */, GenLang language) override;

protected:
    void OnPageChanged(wxBookCtrlEvent& event);
};
