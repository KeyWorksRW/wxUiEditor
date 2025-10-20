/////////////////////////////////////////////////////////////////////////////
// Purpose:   CtxMenuGenerator -- generates function and includes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2023 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include "gen_ctx_menu.h"  // CtxMenuGenerator -- generates function and includes

#include "code.h"          // Code -- Helper class for generating code
#include "node_creator.h"  // NodeCreator class

using namespace GenEnum;

bool CtxMenuGenerator::GetIncludes(Node* /* node */, std::set<std::string>& set_src,
                                   std::set<std::string>&
                                   /* set_hdr */,
                                   GenLang /* language */)
{
    set_src.insert("#include <wx/event.h>");
    set_src.insert("#include <wx/menu.h>");
    set_src.insert("#include <wx/window.h>");

    return true;
}

static void GenCtxConstruction(Code& code)
{
    if (auto generator = code.node()->get_NodeDeclaration()->get_Generator(); generator)
    {
        code.Eol(eol_if_needed);
        generator->ConstructionCode(code);
        generator->SettingsCode(code);
        if (code.node()->is_Gen(gen_submenu))
        {
            code.Eol(eol_if_needed);
            generator->AfterChildrenCode(code);
        }
    }
}

void CtxMenuGenerator::CollectCtxMenuEventHandlers(Node* node, std::vector<NodeEvent*>& events)
{
    ASSERT(node);
    for (auto& iter: node->get_MapEvents())
    {
        if (iter.second.get_value().size())
        {
            events.push_back(&iter.second);
        }
    }

    for (const auto& child: node->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            for (const auto& ctx_child: child->get_ChildNodePtrs())
            {
                CollectCtxMenuEventHandlers(ctx_child.get(), m_CtxMenuEvents);
            }
        }
    }
}

bool CtxMenuGenerator::AfterChildrenCode(Code& code)
{
    if (code.is_cpp())
    {
        code.Str("void ").Str(code.node()->get_FormName()).Str("::").as_string(prop_handler_name);
        code.Str("(wxContextMenuEvent& event)").OpenBrace();
    }

    if (code.is_cpp())
    {
        code.Add("wxMenu ctx_menu").Str(code.is_cpp() ? ";" : "");
        code.Eol().Str(
            "auto* p_ctx_menu = &ctx_menu;  // convenience variable for the auto-generated code");
    }
    else
    {
        code.Str("ctx_menu = wx.Menu");
    }
    code.Eol();

    // All of the constructors are expecting a wxMenu parent -- so we need to temporarily create one
    auto node_menu = NodeCreation.NewNode(NodeCreation.get_NodeDeclaration("wxMenu"));
    node_menu->set_value(prop_var_name, code.is_cpp() ? "p_ctx_menu" : "ctx_menu");

    for (const auto& child: code.node()->get_ChildNodePtrs())
    {
        auto child_node = NodeCreation.MakeCopy(child);
        node_menu->AdoptChild(child_node);
        auto* save_node = code.node();
        code.set_node(child_node.get());
        code.Eol(eol_if_needed);
        GenCtxConstruction(code);
        code.set_node(save_node);
    }
    code.Eol().Eol();
    m_CtxMenuEvents.clear();

    for (const auto& child: code.node()->get_Parent()->get_ChildNodePtrs())
    {
        if (child->is_Gen(gen_wxContextMenuEvent))
        {
            for (const auto& ctx_child: child->get_ChildNodePtrs())
            {
                CollectCtxMenuEventHandlers(ctx_child.get(), m_CtxMenuEvents);
            }
        }
    }

    for (auto& iter: m_CtxMenuEvents)
    {
        if (auto generator = iter->getNode()->get_NodeDeclaration()->get_Generator(); generator)
        {
            Code event_code(iter->getNode(), code.get_language());
            if (generator->GenEvent(event_code, iter,
                                    code.node()->get_ParentName(code.get_language()).as_str());
                event_code.size())
            {
                code.Eol(eol_if_needed).Str("ctx_menu.") += event_code.GetCode();
            }
        }
    }

    code.Eol().Eol();
    if (code.is_cpp())
    {
        code += "wxStaticCast(event.GetEventObject(), wxWindow)->PopupMenu(&ctx_menu);";
        code.CloseBrace();
    }

    return true;
}
