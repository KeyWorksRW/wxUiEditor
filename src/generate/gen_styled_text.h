/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxStyledText (scintilla) generate
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2021 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class StyledTextGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;
    bool SettingsCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;

    void ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop) override;

    int GenXrcObject(Node* node, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node* /* node */, std::set<std::string>& /* handlers */) override;

    bool GetImports(Node* /* node */, std::set<std::string>& /* set_imports */,
                    GenLang language) override;

private:
    void UpdateCustomTypeState(wxPropertyGridManager* prop_grid, std::string_view custom_type_value,
                               bool margin_enabled);
};
