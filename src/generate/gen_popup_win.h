/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPopupWindow/wxPopupTransientWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PopupWinBaseGenerator : public BaseGenerator
{
public:
    bool SettingsCode(Code&) override;
    bool AfterChildrenCode(Code&) override;
    bool HeaderCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     GenLang /* language */) override;
    std::pair<bool, wxue::string> isLanguageVersionSupported(GenLang language) override;
};

class PopupWinGenerator : public PopupWinBaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    wxue::string GetHelpText(Node*) override { return wxue::string("wxPopupWindow"); }
    wxue::string GetHelpURL(Node*) override { return wxue::string("wx_popup_window.html"); }

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};

class PopupTransientWinGenerator : public PopupWinBaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    wxue::string GetHelpText(Node*) override { return wxue::string("wxPopupTransientWindow"); }
    wxue::string GetHelpURL(Node*) override { return wxue::string("wx_popup_transient_window.html"); }

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
};
