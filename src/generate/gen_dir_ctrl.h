/////////////////////////////////////////////////////////////////////////////
// Purpose:   wxGenericDirCtrl generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

class GenericDirCtrlGenerator : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;

    std::optional<ttlib::cstr> GenConstruction(Node* node) override;
    std::optional<ttlib::cstr> GenEvents(NodeEvent* event, const std::string& class_name) override;
    std::optional<ttlib::cstr> GenSettings(Node* node, size_t& auto_indent) override;

    ttlib::cstr GetHelpText(Node*) override { return ttlib::cstr("wxGenericDirCtrl"); }
    ttlib::cstr GetHelpURL(Node*) override { return ttlib::cstr("wx_generic_dir_ctrl.html"); }

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, bool /* add_comments */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};
