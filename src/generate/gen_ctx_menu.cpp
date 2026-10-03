/////////////////////////////////////////////////////////////////////////////
// Purpose:   CtxMenuGenerator -- generates function and includes
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [10-03-2026]

#include "gen_ctx_menu.h"  // CtxMenuGenerator -- generates function and includes

#include "code.h"          // Code -- Helper class for generating code
#include "node_creator.h"  // NodeCreator class
#include <tuple>           // for std::ignore

using namespace GenEnum;

bool CtxMenuGenerator::GetIncludes(Node* /* node */, std::set<std::string>& set_src,
                                   std::set<std::string>&
                                   /* set_hdr */,
                                   GenLang language)
{
    // This provider emits C++ system includes only; nothing is ever placed in the header, and the
    // context menu itself has no generated support in the non-C++ languages.
    if (language != GenLang::cplusplus)
    {
        return false;
    }

    set_src.insert("#include <wx/event.h>");
    set_src.insert("#include <wx/menu.h>");
    set_src.insert("#include <wx/window.h>");

    return true;
}

static void GenCtxConstruction(Code& code)
{
    const NodeDeclaration* node_declaration = code.node()->get_NodeDeclaration();
    if (!node_declaration)
    {
        return;
    }

    if (auto* generator = node_declaration->get_Generator(); generator)
    {
        code.Eol(eol_if_needed);
        generator->ConstructionCode(code);
        std::ignore = generator->SettingsCode(code);
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
    if (!node)
    {
        return;
    }

    // get_MapEvents() returns a reference to the node's member map, so the NodeEvent pointers
    // stored here stay valid for as long as the node itself does.
    for (auto& iter: node->get_MapEvents())
    {
        if (!iter.second.get_value().empty())
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
                CollectCtxMenuEventHandlers(ctx_child.get(), events);
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
        code.Add("wxMenu ctx_menu;");

        // The convenience pointer is only referenced by the adopted children, so omit it when
        // there are no children to avoid an unused-variable warning in the generated code.
        if (!code.node()->get_ChildNodePtrs().empty())
        {
            code.Eol().Str("auto* p_ctx_menu = &ctx_menu;  // convenience variable for the "
                           "auto-generated code");
        }
    }
    else
    {
        code.Str("ctx_menu = ").Object("wxMenu").EndFunction();
    }
    code.Eol();

    // All of the constructors are expecting a wxMenu parent -- so we need to temporarily create one
    NodeDeclaration* menu_declaration = NodeCreation.get_NodeDeclaration("wxMenu");
    if (!menu_declaration)
    {
        return false;
    }

    const NodeSharedPtr node_menu = NodeCreation.NewNode(menu_declaration);
    node_menu->set_value(prop_var_name, code.is_cpp() ? "p_ctx_menu" : "ctx_menu");

    for (const auto& child: code.node()->get_ChildNodePtrs())
    {
        const NodeSharedPtr child_node = NodeCreation.MakeCopy(child);
        if (!child_node)
        {
            continue;
        }

        node_menu->AdoptChild(child_node);
        Node* save_node = code.node();
        code.set_node(child_node.get());
        code.Eol(eol_if_needed);
        GenCtxConstruction(code);
        code.set_node(save_node);
    }
    code.Eol().Eol();
    m_CtxMenuEvents.clear();

    // code.node() is the wxContextMenuEvent node, which is always a direct child of the form,
    // so its parent's children are searched for the event node that owns these handlers.
    Node* parent_node = code.node()->get_Parent();
    if (parent_node)
    {
        for (const auto& child: parent_node->get_ChildNodePtrs())
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

    for (auto& iter: m_CtxMenuEvents)
    {
        if (auto* generator = iter->getNode()->get_NodeDeclaration()->get_Generator(); generator)
        {
            Code event_code(iter->getNode(), code.get_language());
            const std::string parent_name(code.node()->get_ParentName(code.get_language()));
            if (generator->GenEvent(event_code, iter, parent_name); !event_code.empty())
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
    else if (code.is_python())
    {
        code.Str("event.GetEventObject().PopupMenu(ctx_menu)");
    }
    else if (code.is_ruby())
    {
        code.Str("event.get_event_object.popup_menu(ctx_menu)");
    }

    return true;
}
