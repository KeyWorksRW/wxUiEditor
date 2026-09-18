/////////////////////////////////////////////////////////////////////////////
// Purpose:   Undo/Redo stack information dialog class
// Author:    Ralph Walden
// Copyright: Copyright (c) 2023-2026 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#include <format>

#include "undo_info.h"

#include "mainframe.h"
#include "node.h"
#include "node_info.h"
#include "node_prop.h"
#include "undo_stack.h"

UndoInfo::UndoInfo() {}

UndoInfo::UndoInfo(wxWindow* parent)
{
    Create(parent);
}

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
        ++node_memory.children;
        if (node.use_count() <= ref_count)
        {
            node_memory.size += node->get_NodeSize();
        }

        for (const auto& iter: node->get_ChildNodePtrs())
        {
            // Assume that each child will have a shared ptr to the parent which will increase
            // it's reference count by 1.
            long add_ref_count = static_cast<long>(iter->get_ChildCount());

            // An orphaned node will have a ref count of 1 -- add one to pass this to the
            // CalcMemory function.
            CalcMemory(iter, add_ref_count + 2, CalcMemory);
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
                    CalcMemory(old_node, 9999, CalcMemory);
                }
                else
                {
                    // Assume that each child will have a shared ptr to the parent which will
                    // increase it's reference count by 1.
                    long add_ref_count = static_cast<long>(old_node->get_ChildCount());

                    CalcMemory(old_node, add_ref_count + 3, CalcMemory);
                }
                node_memory.size += iter->GetMemorySize();
            }
            else if (const auto& node = iter->getNode(); node)
            {
                // An orphaned node will have an additional 2 reference counts at this point. 1 for
                // iter->getNode() in the function that called us, and one for passing the parameter
                // to this function. An additional ref count is added by calling CalcMemory.
                CalcMemory(node, 3, CalcMemory);
                node_memory.size += iter->GetMemorySize();
            }
            else if (const auto* prop = iter->GetProperty(); prop)
            {
                node_memory.size += iter->GetMemorySize();
            }
        }

        auto txt_items = std::format(std::locale(""), "{:L}", actions.size());
        ptxt_items->SetLabel(txt_items);

        if (node_memory.size > 0)
        {
            auto txt_totals =
                std::format(std::locale(""), "{:L} ({:L} node{})", node_memory.size,
                            node_memory.children, node_memory.children == 1 ? "" : "s");
            ptxt_memory->SetLabel(txt_totals);
        }
        else
        {
            ptxt_memory->SetLabel("0");
        }
    };

    const auto& undo_stack = wxGetMainFrame()->getUndoStack();
    const auto& undo_vector = undo_stack.GetUndoVector();
    const auto& redo_vector = undo_stack.GetRedoVector();

    ParseActions(undo_vector, m_txt_undo_items, m_txt_undo_memory);
    ParseActions(redo_vector, m_txt_redo_items, m_txt_redo_memory);

    Fit();

    event.Skip();
}
