/////////////////////////////////////////////////////////////////////////////
// Purpose:   Node memory usage dialog
// Author:    Ralph Walden
// Copyright: Copyright (c) 2020-2022 KeyWorks Software (Ralph Walden)
// License:   Apache License -- see ../../LICENSE
/////////////////////////////////////////////////////////////////////////////

#if !defined(INTERNAL_TESTING)
    #error "INTERNAL_TESTING must be defined if you include this moduel!"
#endif

#include "node_info.h"  // auto-generated: nodeinfo_base.h and nodeinfo_base.cpp

#include "mainapp.h"        // App -- Main application class
#include "mainframe.h"      // Main window frame
#include "node.h"           // Node class
#include "project_class.h"  // Project class
#include "undo_stack.h"     // UndoAction -- Maintain a undo and redo stack

struct NodeMemory
{
    size_t size { 0 };
    size_t children { 0 };
};

static void CalcNodeMemory(Node* node, NodeMemory& node_memory)
{
    node_memory.size += node->GetNodeSize();
    ++node_memory.children;

    for (auto& iter: node->GetChildNodePtrs())
    {
        CalcNodeMemory(iter.get(), node_memory);
    }
}

// If this constructor is used, the caller must call Create(parent)
NodeInfo::NodeInfo() {}

NodeInfo::NodeInfo(wxWindow* parent)
{
    Create(parent);
}

void NodeInfo::OnInit(wxInitDialogEvent& /* event */)
{
    ttlib::cstr label;
    NodeMemory node_memory;

    auto* cur_sel = m_node ? m_node : wxGetFrame().GetSelectedNode();
    if (cur_sel)
    {
        label.clear();
        label << "Generator: gen_" << cur_sel->DeclName();
        m_txt_generator->SetLabel(label);
        label.clear();
        label << "Type: type_" << GenEnum::map_GenTypes.at(cur_sel->gen_type());
        m_txt_type->SetLabel(label);

        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(cur_sel, node_memory);
        label.clear();
        label.Format("Memory: %kzu (%kzu node%s)", node_memory.size, node_memory.children,
                     node_memory.children == 1 ? "" : "s");
        m_txt_memory->SetLabel(label);
    }

    auto project = GetProject();
    CalcNodeMemory(project, node_memory);

    label.Format("Project: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
    m_txt_project->SetLabel(label);

    auto clipboard = wxGetFrame().GetClipboard();
    if (clipboard)
    {
        node_memory.size = 0;
        node_memory.children = 0;
        CalcNodeMemory(clipboard, node_memory);
        label.clear();
        label.Format("Clipboard: %kzu (%kzu nodes)", node_memory.size, node_memory.children);
        m_txt_clipboard->SetLabel(label);
    }

    Fit();
}
