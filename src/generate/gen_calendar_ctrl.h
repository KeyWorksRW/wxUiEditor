/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxCalendarCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class CalendarCtrlGenerator : public BaseGenerator
{
public:
    auto CreateMockup(Node* node, wxObject* parent) -> wxObject* override;

    auto ConstructionCode(Code& code) -> bool override;
    auto SettingsCode(Code& /*unused*/) -> bool override;

    auto GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) -> bool override;
    auto GetPythonImports(Node* /*unused*/, std::set<std::string>& set_imports) -> bool override;

    auto GenXrcObject(Node* /*unused*/, pugi::xml_node& /* object */, size_t /* xrc_flags */)
        -> int override;
    void RequiredHandlers(Node* /*unused*/, std::set<std::string>& /* handlers */) override;

    auto GetPythonURL(Node* /*node*/) -> wxue::string override
    {
        return "wx.adv.CalendarCtrl.html";
    }

    auto GetImports(Node* /*unused*/, std::set<std::string>& set_imports, GenLang language)
        -> bool override;
};
