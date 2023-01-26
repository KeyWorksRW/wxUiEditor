//////////////////////////////////////////////////////////////////////////
// Purpose:   wxBitmapButton::CreateCloseButton generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class CloseButtonGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    bool ConstructionCode(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    tt_string GetHelpText(Node*) override { return tt_string("CreateCloseButton"); }
    tt_string GetHelpURL(Node*) override { return tt_string("wx_bitmap_button.html#a558bf8e66279a784260950d9e585baf7"); }

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;

    tt_string GetPythonHelpText(Node*) override { return "NewCloseButton"; }
    tt_string GetPythonURL(Node*) override { return "wx.BitmapButton.html#wx.BitmapButton.NewCloseButton"; }
};
