/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undo/Redo stack information dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////
// CR: [09-19-2026]

#include <format>
#include <locale>
#include <stdexcept>
#include <tuple>  // for std::ignore

#include "undo_info.h"

#include "mainframe.h"
#include "node.h"
#include "node_info.h"
#include "node_prop.h"
#include "undo_stack.h"

UndoInfo::UndoInfo() {}

UndoInfo::UndoInfo(wxWindow* parent)
{
    std::ignore = Create(parent);
}

// Baseline reference counts used to decide whether a Node is only being held by the UndoStack.
// These encode the UndoStack baseline refs, each child's back-pointer to its parent, and the
// temporary parameter copies made while recursing. A change to Node's ownership model would
// silently invalidate these values.
static constexpr long k_project_always_counted = 9999;
static constexpr long k_orphan_base_refs = 2;
static constexpr long k_child_base_refs = 3;

void UndoInfo::OnInit(wxInitDialogEvent& event)
{
    struct NodeMemory
    {
        size_t size { 0 };
        size_t children { 0 };
    };

    NodeMemory node_memory;

    // The problem with getting the memory size is that it's a bit tricky to know what the
    // reference count needs to be under to indicate that a Node is only being held by the
    // UndoStack.

    // The auto&& CalcMemory and forced return type is so that we can recursively call this
    // lambda function.
    auto CalcMemory = [&node_memory](const NodeSharedPtr node, long ref_count,
                                     auto&& CalcMemory) -> void
    {
        if (!node)
        {
            return;
        }

        ++node_memory.children;
        if (node.use_count() <= ref_count)
        {
            node_memory.size += node->get_NodeSize();
        }

        for (const auto& iter: node->get_ChildNodePtrs())
        {
            // Assume that each child will have a shared ptr to the parent which will increase
            // it's reference count by 1.
            const long add_ref_count = static_cast<long>(iter->get_ChildCount());

            // An orphaned node will have a ref count of 1 -- add one to pass this to the
            // CalcMemory function.
            CalcMemory(iter, add_ref_count + k_orphan_base_refs, CalcMemory);
        }
    };

    // This will iterate through the vector of actions, adding up the memory size (and possible
    // number of node children) for each action in the vector.
    auto ParseActions = [&](const std::vector<UndoActionPtr>& actions, wxStaticText* ptxt_items,
                            wxStaticText* ptxt_memory)
    {
        node_memory.size = 0;
        node_memory.children = 0;

        for (const auto& iter: actions)
        {
            if (const auto& old_node = iter->GetOldNode(); old_node)
            {
                if (old_node->is_Gen(gen_Project))
                {
                    // Every form in the project will increase the project's ref count
                    CalcMemory(old_node, k_project_always_counted, CalcMemory);
                }
                else
                {
                    // Assume that each child will have a shared ptr to the parent which will
                    // increase it's reference count by 1.
                    const long add_ref_count = static_cast<long>(old_node->get_ChildCount());

                    CalcMemory(old_node, add_ref_count + k_child_base_refs, CalcMemory);
                }
                node_memory.size += iter->GetMemorySize();
            }
            else if (const auto& node = iter->getNode(); node)
            {
                // An orphaned node will have an additional 2 reference counts at this point. 1 for
                // iter->getNode() in the function that called us, and one for passing the parameter
                // to this function. The child back-pointer refs plus the base refs added by calling
                // CalcMemory are counted as well.
                CalcMemory(node, static_cast<long>(node->get_ChildCount()) + k_child_base_refs,
                           CalcMemory);
                node_memory.size += iter->GetMemorySize();
            }
            else if (const auto* prop = iter->GetProperty(); prop)
            {
                node_memory.size += iter->GetMemorySize();
            }
        }

        // Constructing a locale from the user environment can throw; fall back to the global
        // locale so a bad environment can't escape OnInit.
        std::locale user_locale;
        try
        {
            user_locale = std::locale("");
        }
        catch (const std::runtime_error&)
        {
            user_locale = std::locale();
        }

        const std::string txt_items = std::format(user_locale, "{:L}", actions.size());
        ptxt_items->SetLabel(txt_items);

        const std::string txt_totals =
            std::format(user_locale, "{:L} ({:L} node{})", node_memory.size, node_memory.children,
                        node_memory.children == 1 ? "" : "s");
        ptxt_memory->SetLabel(txt_totals);
    };

    const UndoStack& undo_stack = wxGetMainFrame()->getUndoStack();
    const std::vector<UndoActionPtr>& undo_vector = undo_stack.GetUndoVector();
    const std::vector<UndoActionPtr>& redo_vector = undo_stack.GetRedoVector();

    ParseActions(undo_vector, m_txt_undo_items, m_txt_undo_memory);
    ParseActions(redo_vector, m_txt_redo_items, m_txt_redo_memory);

    Fit();

    event.Skip();
}
