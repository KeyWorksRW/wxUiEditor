/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>               // wxMenu and wxMenuBar classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_common.h"    // GeneratorLibrary -- Generator classes
#include "mainframe.h"     // MainFrame -- Main window frame
#include "node.h"          // Node class
#include "node_creator.h"  // NodeCreator -- NodeCreator class
#include "undo_cmds.h"     // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "gen_menu.h"

bool MenuGenerator::ConstructionCode(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass().EndFunction();

    return true;
}

bool MenuGenerator::AdditionalCode(Code& code, GenEnum::GenCodeType cmd)
{
    if (cmd == code_after_children)
    {
        Node* node = code.node();
        auto parent_type = node->GetParent()->gen_type();
        if (parent_type == type_menubar)
        {
            code.ParentName().Function("Append(").NodeName().Comma();
            if (node->value(prop_stock_id) != "none")
            {
                code.Add("wxGetStockLabel(").Add(node->value(prop_stock_id)).Str(")");
            }
            else
            {
                code.QuotedString(prop_label);
            }
            code.EndFunction();
        }
        else if (parent_type == type_menubar_form)
        {
            if (code.is_python())
                code += "self.";
            code.Add("Append(").NodeName().Comma().QuotedString(prop_label).EndFunction();
        }
        else if (code.is_cpp())
        {
            // The parent can disable generation of Bind by shutting off the context menu
            if (!node->GetParent()->as_bool(prop_context_menu))
            {
                return true;
            }

            if (parent_type == type_form || parent_type == type_frame_form || parent_type == type_wizard)
            {
                code << "Bind(wxEVT_RIGHT_DOWN, &" << node->get_parent_name() << "::" << node->get_parent_name()
                     << "OnContextMenu, this);";
            }
            else
            {
                code.ValidParentName().Function("Bind(wxEVT_RIGHT_DOWN, &")
                    << node->get_form_name() << "::" << node->get_parent_name() << "OnContextMenu, this);";
            }
        }
    }

    return true;
}

bool MenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src, std::set<std::string>& set_hdr)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_menu.cpp
// ../../../wxWidgets/src/xrc/xh_menu.cpp

int MenuGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    auto item = InitializeXrcObject(node, object);

    GenXrcObjectAttributes(node, item, "wxMenu");

    ADD_ITEM_PROP(prop_label, "label")
    GenXrcBitmap(node, item, xrc_flags);

    return BaseGenerator::xrc_updated;
}

void MenuGenerator::RequiredHandlers(Node* /* node */, std::set<std::string>& handlers)
{
    handlers.emplace("wxMenuXmlHandler");
}

void MenuGenerator::ChangeEnableState(wxPropertyGridManager* prop_grid, NodeProperty* changed_prop)
{
    if (changed_prop->isProp(prop_stock_id))
    {
        if (auto pg_setting = prop_grid->GetProperty(map_PropNames[prop_label]); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_string() == "none");
        }
    }
}

bool MenuGenerator::ModifyProperty(NodeProperty* prop, tt_string_view value)
{
    if (prop->isProp(prop_stock_id))
    {
        if (value != "none")
        {
            auto undo_stock_id = std::make_shared<ModifyProperties>("Stock ID");
            undo_stock_id->AddProperty(prop, value);
            undo_stock_id->AddProperty(prop->GetNode()->get_prop_ptr(prop_label),
                                       wxGetStockLabel(NodeCreation.GetConstantAsInt(value.as_str())).utf8_string());
            wxGetFrame().PushUndoAction(undo_stock_id);
            return true;
        }
    }
    return false;
}
