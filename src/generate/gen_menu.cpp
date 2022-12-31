/////////////////////////////////////////////////////////////////////////////
// Purpose:   Menu Generator
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <wx/menu.h>  // wxMenu and wxMenuBar classes

#include "gen_common.h"  // GeneratorLibrary -- Generator classes
#include "node.h"        // Node class

#include "gen_menu.h"

std::optional<ttlib::sview> MenuGenerator::CommonConstruction(Code& code)
{
    if (code.is_cpp() && code.is_local_var())
        code << "auto* ";
    code.NodeName().CreateClass().EndFunction();

    return code.m_code;
}

std::optional<ttlib::sview> MenuGenerator::CommonAdditionalCode(Code& code, GenEnum::GenCodeType cmd)
{
    if (cmd == code_after_children)
    {
        Node* node = code.node();
        auto parent_type = node->GetParent()->gen_type();
        if (parent_type == type_menubar)
        {
            code.ParentName().Function("Append(").NodeName().Comma().QuotedString(prop_label).EndFunction();
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
                return code.m_code;
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

    return code.m_code;
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
