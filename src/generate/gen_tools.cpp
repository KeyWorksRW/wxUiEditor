/////////////////////////////////////////////////////////////////////////////
// Purpose:   Generate tools for wxToolBar
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2025 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/toolbar.h>  // wxToolBar interface declaration

#include "gen_toolbar.h"

#include "code.h"           // Code -- Helper class for generating code
#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mockup_parent.h"  // Top-level MockUp Parent window
#include "node.h"           // Node class
#include "node_creator.h"   // Class used to create nodes
#include "utils.h"          // Utility functions that work with properties

///////////////////////////////////  ToolDropDownGenerator //////////////////////////////////////

bool ToolDropDownGenerator::ConstructionCode(Code& code)
{
    GenToolCode(code);
    return true;
}

bool ToolDropDownGenerator::SettingsCode(Code& code)
{
    tt_string menu_name = code.node()->as_string(prop_var_name);
    menu_name += "_menu";
    code.AddIfCpp("auto* ").Str(menu_name).Assign("wxMenu");
    code.AddIfPython("()");
    auto menu_node_ptr = NodeCreation.newNode(gen_wxMenu);
    menu_node_ptr->setParent(code.node());  // Python code generation needs this set
    menu_node_ptr->set_value(prop_var_name, menu_name);
    menu_node_ptr->set_value(prop_class_access, "none");

    for (const auto& child: code.node()->getChildNodePtrs())
    {
        auto old_parent = child->getParent();
        child->setParent(menu_node_ptr.get());
        if (auto gen = child->getNodeDeclaration()->getGenerator(); gen)
        {
            Code child_code(child.get(), code.m_language);
            if (gen->ConstructionCode(child_code))
            {
                code.Eol() += child_code;
                child_code.clear();
                if (gen->SettingsCode(child_code) && child_code.size())
                {
                    code.Eol() += child_code;
                }
            }
        }

        // A submenu can have children
        if (child->getChildCount())
        {
            for (const auto& grandchild: child->getChildNodePtrs())
            {
                if (auto gen = grandchild->getNodeDeclaration()->getGenerator(); gen)
                {
                    Code child_code(grandchild.get(), code.m_language);
                    if (gen->ConstructionCode(child_code))
                    {
                        code.Eol() += child_code;
                        child_code.clear();
                        if (gen->SettingsCode(child_code) && child_code.size())
                        {
                            code.Eol() += child_code;
                        }
                    }
                }
                // A submenu menu item can also be a submenu with great grandchildren.
                if (grandchild->getChildCount())
                {
                    for (const auto& great_grandchild: grandchild->getChildNodePtrs())
                    {
                        if (auto gen = great_grandchild->getNodeDeclaration()->getGenerator(); gen)
                        {
                            Code child_code(great_grandchild.get(), code.m_language);
                            if (gen->ConstructionCode(child_code))
                            {
                                code.Eol() += child_code;
                                child_code.clear();
                                if (gen->SettingsCode(child_code) && child_code.size())
                                {
                                    code.Eol() += child_code;
                                }
                            }
                        }
                        // It's possible to have even more levels of submenus, but we'll stop here.
                    }
                }
            }
        }
        child->setParent(old_parent);
    }
    code.Eol().NodeName().Function("SetDropdownMenu(").Str(menu_name).EndFunction();

    return true;
}

int ToolDropDownGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    if (node->getChildCount())
    {
        object = object.append_child("dropdown");
        object = object.append_child("object");
        object.append_attribute("class").set_value("wxMenu");

        for (const auto& child: node->getChildNodePtrs())
        {
            auto child_object = object.append_child("object");
            auto child_generator = child->getNodeDeclaration()->getGenerator();
            if (child_generator->GenXrcObject(child.get(), child_object, xrc_flags) ==
                BaseGenerator::xrc_not_supported)
            {
                object.remove_child(child_object);
            }

            // A submenu can have children
            if (child->getChildCount())
            {
                for (const auto& grandchild: child->getChildNodePtrs())
                {
                    auto grandchild_object = child_object.append_child("object");
                    auto grandchild_generator = grandchild->getNodeDeclaration()->getGenerator();
                    if (grandchild_generator->GenXrcObject(grandchild.get(), grandchild_object,
                                                           xrc_flags) ==
                        BaseGenerator::xrc_not_supported)
                    {
                        child_object.remove_child(grandchild_object);
                    }
                    // A submenu menu item can also be a submenu with great grandchildren.
                    if (grandchild->getChildCount())
                    {
                        for (const auto& great_grandchild: grandchild->getChildNodePtrs())
                        {
                            auto great_grandchild_object = grandchild_object.append_child("object");
                            auto great_grandchild_generator =
                                grandchild->getNodeDeclaration()->getGenerator();
                            if (great_grandchild_generator->GenXrcObject(
                                    great_grandchild.get(), great_grandchild_object, xrc_flags) ==
                                BaseGenerator::xrc_not_supported)
                            {
                                grandchild_object.remove_child(grandchild_object);
                            }
                            // It's possible to have even more levels of submenus, but we'll stop
                            // here.
                        }
                    }
                }
            }
        }
    }

    return BaseGenerator::xrc_updated;
}

//////////////////////////////////////////  ToolGenerator //////////////////////////////////////////

bool ToolGenerator::ConstructionCode(Code& code)
{
    GenToolCode(code);

    if (code.IsTrue(prop_disabled))
    {
        code.Eol().NodeName().Function("Enable(") << (code.is_cpp() ? "false" : "False");
        code.EndFunction();
    }

    return true;
}

int ToolGenerator::GetRequiredVersion(Node* node)
{
    if (node->as_bool(prop_disabled))
    {
        return std::max(minRequiredVer + 1, BaseGenerator::GetRequiredVersion(node));
    }
    return BaseGenerator::GetRequiredVersion(node);
}

int ToolGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);
    GenXrcObjectAttributes(node, item, "tool");
    GenXrcToolProps(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

///////////////////////////////////  ToolSeparatorGenerator //////////////////////////////////////

bool ToolSeparatorGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    if (node->isParent(gen_wxToolBar) || node->isParent(gen_wxRibbonToolBar) ||
        node->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddSeparator(").EndFunction();
    }
    else
    {
        code.FormFunction("AddSeparator(").EndFunction();
    }

    return true;
}

int ToolSeparatorGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& object,
                                         size_t /* xrc_flags */)
{
    object.append_attribute("class").set_value("separator");

    return BaseGenerator::xrc_updated;
}

tt_string ToolSeparatorGenerator::GetHelpURL(Node*)
{
    return "wx_tool_bar.html";
}

tt_string ToolSeparatorGenerator::GetHelpText(Node*)
{
    return "wxToolBar";
}

tt_string ToolSeparatorGenerator::GetPythonURL(Node*)
{
    return "wx.ToolBar.html?highlight=addseparator#wx.ToolBar.AddSeparator";
}

tt_string ToolSeparatorGenerator::GetPythonHelpText(Node*)
{
    return "wx.ToolBar";
}

tt_string ToolSeparatorGenerator::GetRubyURL(Node*)
{
    return "Wx/ToolBar.html#add_separator-instance_method";
}

tt_string ToolSeparatorGenerator::GetRubyHelpText(Node*)
{
    return "Wx/ToolBar.html";
}

//////////////////////////////////  ToolStretchableGenerator /////////////////////////////////////

bool ToolStretchableGenerator::ConstructionCode(Code& code)
{
    auto* node = code.node();
    if (node->isParent(gen_wxToolBar))
    {
        code.ParentName().Function("AddStretchableSpace(").EndFunction();
    }
    else if (node->isParent(gen_wxAuiToolBar))
    {
        code.ParentName().Function("AddStretchSpacer(");
        if (code.IntValue(prop_proportion) != 1)
        {
            code.as_string(prop_proportion);
        }
        code.EndFunction();
    }
    else
    {
        code.FormFunction("AddStretchableSpace(").EndFunction();
    }

    return true;
}

int ToolStretchableGenerator::GenXrcObject(Node* /* node */, pugi::xml_node& object,
                                           size_t /* xrc_flags */)
{
    object.append_attribute("class").set_value("space");

    return BaseGenerator::xrc_updated;
}

tt_string ToolStretchableGenerator::GetHelpURL(Node*)
{
    return "wx_tool_bar.html";
}

tt_string ToolStretchableGenerator::GetHelpText(Node*)
{
    return "wxToolBar";
}

tt_string ToolStretchableGenerator::GetPythonURL(Node*)
{
    return "wx.ToolBar.html?highlight=addstretchablespace#wx.ToolBar.AddStretchableSpace";
}

tt_string ToolStretchableGenerator::GetPythonHelpText(Node*)
{
    return "wx.ToolBar";
}

tt_string ToolStretchableGenerator::GetRubyURL(Node*)
{
    return "Wx/ToolBar.html#add_stretchable_space-instance_method";
}

tt_string ToolStretchableGenerator::GetRubyHelpText(Node*)
{
    return "Wx/ToolBar.html";
}
