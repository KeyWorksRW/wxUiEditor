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
    // bool OnPropertyChange(wxObject* widget, Node* node, NodeProperty* prop) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    // std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    ttlib::cstr GetHelpText(Node*) override { return ttlib::cstr("CreateCloseButton"); }
    ttlib::cstr GetHelpURL(Node*) override { return ttlib::cstr("wx_bitmap_button.html#a558bf8e66279a784260950d9e585baf7"); }

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};
