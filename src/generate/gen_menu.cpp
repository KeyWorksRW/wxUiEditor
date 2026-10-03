/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [10-03-2026]

#include <wx/menu.h>               // wxMenu and wxMenuBar classes
#include <wx/propgrid/manager.h>   // wxPropertyGridManager
#include <wx/propgrid/propgrid.h>  // wxPropertyGrid

#include "gen_common.h"     // GeneratorLibrary -- Generator classes
#include "gen_xrc_utils.h"  // Common XRC generating functions
#include "mainframe.h"      // MainFrame -- Main window frame
#include "node.h"           // Node class
#include "node_creator.h"   // NodeCreator -- NodeCreator class
#include "undo_cmds.h"      // InsertNodeAction -- Undoable command classes derived from UndoAction

#include "gen_menu.h"

bool MenuGenerator::ConstructionCode(Code& code)
{
    code.AddAuto().NodeName().CreateClass().EndFunction();

    return true;
}

bool MenuGenerator::AfterChildrenCode(Code& code)
{
    Node* node =
        code.node();  // This is just for code readability -- could just use code.node() everywhere

    // A Menu node is never nested inside another Menu: nested menus are type_submenu nodes
    // handled by SubMenuGenerator, which emits AppendSubMenu(). Cache the parent once and
    // guard against a null parent before dereferencing it.
    Node* parent = node->get_Parent();
    if (!parent)
    {
        return true;
    }
    const GenType parent_type = parent->get_GenType();
    if (parent_type == type_menubar)
    {
        code.ParentName().Function("Append(").NodeName().Comma();
        if (node->as_string(prop_stock_id) != "none")
        {
            // Pass add_operator=false so no receiver is emitted: C++ gets the bare
            // wxGetStockLabel() call while Ruby converts it to Wx::get_stock_label().
            code.Function("wxGetStockLabel(", false).Add(prop_stock_id).Str(")");
        }
        else
        {
            code.QuotedString(prop_label);
        }
        code.EndFunction();
    }
    else if (parent_type == type_menubar_form)
    {
        // This branch is only valid for targets that supply the receiver implicitly: Python
        // via AddIfPython("self."), and C++ where the menu is appended from inside the
        // wxMenuBar-derived constructor so a bare Append() call resolves to this->Append().
        code.AddIfPython("self.");
        code.Add("Append(").NodeName().Comma().QuotedString(prop_label).EndFunction();
    }
    else if (code.is_cpp())
    {
        // The parent can disable generation of Bind by shutting off the context menu
        if (!parent->as_bool(prop_context_menu))
        {
            return true;
        }

        // The wxEVT_RIGHT_DOWN handler name is the parent node's name with an "OnContextMenu"
        // suffix, matching the wxEVT_CONTEXT_MENU handler name emitted by GenEvent() in
        // CtxMenuGenerator. The class qualifier is always the enclosing form's name.
        if (parent_type == type_form || parent_type == type_frame_form ||
            parent_type == type_panel_form || parent_type == type_wizard)
        {
            code << "Bind(wxEVT_RIGHT_DOWN, &" << node->get_FormName()
                 << "::" << node->get_ParentName(code.get_language()) << "OnContextMenu, this);";
        }
        else
        {
            code.ValidParentName().Function("Bind(wxEVT_RIGHT_DOWN, &")
                << node->get_FormName() << "::" << node->get_ParentName(code.get_language())
                << "OnContextMenu, this);";
        }
    }
    code.Eol(eol_if_needed);

    return true;
}

bool MenuGenerator::GetIncludes(Node* node, std::set<std::string>& set_src,
                                std::set<std::string>& set_hdr, GenLang /* language */)
{
    InsertGeneratorInclude(node, "#include <wx/menu.h>", set_src, set_hdr);

    return true;
}

// ../../wxSnapShot/src/xrc/xh_menu.cpp
// ../../../wxWidgets/src/xrc/xh_menu.cpp

int MenuGenerator::GenXrcObject(Node* node, pugi::xml_node& object, size_t xrc_flags)
{
    pugi::xml_node item = InitializeXrcObject(node, object);

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
        const std::map<GenEnum::PropName, std::string_view>::const_iterator label_iter =
            map_PropNames.find(prop_label);
        if (label_iter == map_PropNames.end())
        {
            return;
        }

        if (auto* pg_setting = prop_grid->GetProperty(wxString(label_iter->second)); pg_setting)
        {
            pg_setting->Enable(changed_prop->as_string() == "none");
        }
    }
}

bool MenuGenerator::ModifyProperty(NodeProperty* prop, wxue::string_view value)
{
    if (prop->isProp(prop_stock_id))
    {
        if (value != "none")
        {
            auto undo_stock_id = std::make_shared<ModifyProperties>("Stock ID");
            undo_stock_id->addProperty(prop, value);
            // Only update the label when value names a known stock id; otherwise
            // wxGetStockLabel() would be called with a bogus id.
            if (const int stock_id = NodeCreation.get_ConstantAsInt(value.as_str()); stock_id > 0)
            {
                undo_stock_id->addProperty(prop->getNode()->get_PropPtr(prop_label),
                                           wxGetStockLabel(stock_id).utf8_string());
            }
            wxGetFrame().PushUndoAction(undo_stock_id);
            return true;
        }
    }
    return false;
}
