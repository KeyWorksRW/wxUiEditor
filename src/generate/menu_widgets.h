/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu component classes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
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
    tt_string GetHelpText(Node*) override { return tt_string("wxMenuBar"); }
    tt_string GetHelpURL(Node*) override { return tt_string("wx_menu_bar.html"); }

protected:
    wxMenu* MakeSubMenu(Node* node);
    void OnLeftMenuClick(wxMouseEvent& event);

private:
    Node* m_node_menubar;
};

class MenuBarGenerator : public MenuBarBase
{
public:
    bool ConstructionCode(Code& code) override;
    bool AdditionalCode(Code&, GenEnum::GenCodeType /* command */) override;

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
    bool ConstructionCode(Code&) override;

    bool GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr) override;

    int GenXrcObject(Node*, pugi::xml_node& /* object */, size_t /* xrc_flags */) override;

    tt_string GetPythonHelpText(Node*) override { return "wx.Menu.AppendSeparator"; }
    tt_string GetPythonURL(Node*) override { return "wx.Menu.html#wx.Menu.AppendSeparator"; }
};
