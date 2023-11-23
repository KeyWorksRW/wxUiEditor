/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxPopupTransientWindow generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class PopupWinGenerator : public BaseGenerator
{
public:
    bool ConstructionCode(Code&) override;
    bool SettingsCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    tt_string GetHelpText(Node*) override { return tt_string("wxPopupTransientWindow"); }
    tt_string GetHelpURL(Node*) override { return tt_string("wx_popup_transient_window.html"); }

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr,
                     int /* language */) override;
};
