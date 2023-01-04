/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "base_generator.h"  // BaseGenerator -- Base Generator class

#include "gen_menu.h"      // MenuGenerator -- Menu generator
#include "gen_menuitem.h"  // MenuItemGenerator -- Menu Item generator
#include "gen_submenu.h"   // SubMenu Generator

// MenuBarGenerator, MenuBarFormGenerator and PopupMenuGenerator are derived from this class
class MenuBarBase : public BaseGenerator
{
public:
    wxObject* CreateMockup(Node* node, wxObject* parent) override;
    ttlib::cstr GetHelpText(Node*) override { return ttlib::cstr("wxMenuBar"); }
    ttlib::cstr GetHelpURL(Node*) override { return ttlib::cstr("wx_menu_bar.html"); }

protected:
    wxMenu* MakeSubMenu(Node* node);
    void OnLeftMenuClick(wxMouseEvent& event);

private:
    Node* m_node_menubar;
};

class MenuBarGenerator : public MenuBarBase
{
public:
    std::optional<ttlib::sview> CommonConstruction(Code& code) override;
    std::optional<ttlib::sview> CommonAdditionalCode(Code&, GenEnum::GenCodeType /* command */) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class MenuBarFormGenerator : public MenuBarBase
{
public:
    bool ConstructionCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;
    void RequiredHandlers(Node*, std::set<std::string>& /* handlers */) override;
};

class PopupMenuGenerator : public MenuBarBase
{
public:
    bool ConstructionCode(Code&) override;
    bool HeaderCode(Code&) override;
    bool BaseClassNameCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};

class SeparatorGenerator : public BaseGenerator
{
public:
    std::optional<ttlib::sview> CommonConstruction(Code& code) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    ttlib::cstr GetPythonHelpText(Node*) override { return "wx.Menu.AppendSeparator"; }
    ttlib::cstr GetPythonURL(Node*) override { return "wx.Menu.html#wx.Menu.AppendSeparator"; }
};

class CtxMenuGenerator : public BaseGenerator
{
public:
    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;
};
